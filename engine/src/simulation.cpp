/*
 * Copyright 2024 Robert Bosch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file simulation.cpp
 * \see  simulation.hpp
 *
 * This file provides the simulation state machine.
 *
 * The following flow diagram shows how the states of a simulation are
 * traversed in a typical simulation. The nominal flow is rendered in solid
 * lines, while irregular situations are rendered in dashed lines.
 *
 *                         ┌──────────────────────┐
 *           +------------ │       Connect        │
 *           |             └──────────────────────┘
 *           |                        │
 *           |                        ▼
 *           |             ┌──────────────────────┐
 *           +---...       │        Start         │ <-------------------------+
 *           |             └──────────────────────┘                           |
 *           |                        │                                       |
 *           |                        ▼                                       |
 *           |             ┌──────────────────────┐          +-----------+    |
 *           +---...       │      StepBegin       │ ◀──┐<--- |   Resume  |    |
 *           |             └──────────────────────┘    │     +-----------+    |
 *           |                        │                │           ^          |
 *           |                        ▼                │           |          |
 *           |             ┌──────────────────────┐    │           |          |
 *           +---...       │    StepSimulators    │    │           |          |
 *           |             └──────────────────────┘    │           |          |
 *           |                        │                │           |          |
 *           |                        ▼                │           |          |
 *           |             ┌──────────────────────┐    │           |          |
 *           +---...       │    StepControllers   │    │           |          |
 *           |             └──────────────────────┘    │           |          |
 *           |                        │                │           |          |
 *           v                        ▼                │           |          |
 *     +-----------+       ┌──────────────────────┐    │     +-----------+    |
 *     |   Abort   |       │       StepEnd        │ ───┘---> |   Pause   |    |
 *     +-----------+       └──────────────────────┘          +-----------+    |
 *         |    |                     │                         |     ^       |
 *         |    |             failure │ success                 |     |       |
 *         |    |                     ▼                         +-----+       |
 *         |    |          ┌──────────────────────┐          +-----------+    |
 *         |    +--------> │        Stop          │ -------> |   Reset   | ---+
 *         |               └──────────────────────┘          +-----------+
 *         |                          │
 *         |                          ▼
 *         |               ┌──────────────────────┐
 *         +-------------> │      Disconnect      │
 *                         └──────────────────────┘
 *
 * Note that not all possible transitions or states are presented in the above
 * diagram; for example, it is possible to go into the Abort state from almost
 * any other state. Neither can one see the constraints that apply to the above
 * transitions; for example, after Abort, the state machine may go into the
 * Stop state, but then will in every case go into the Disconnect state and
 * never into the Reset state.
 */

#include "simulation.hpp"

#include <filesystem>  // for filesystem::path
#include <fstream>     // for ofstream
#include <string>      // for string
#include <vector>      // for vector<>

#include <cloe/data_broker.hpp>   // for DataBroker
#include <fable/utility.hpp>      // for pretty_print
#include <fable/utility/sol.hpp>  // for sol::object to_json

#include "coordinator.hpp"         // for Coordinator usage
#include "lua_api.hpp"             // for luat_cloe_engine_state
#include "server.hpp"              // for Server usage
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine
#include "simulation_probe.hpp"    // for SimulationProbe
#include "simulation_result.hpp"   // for SimulationResult
#include "utility/command.hpp"     // for CommandExecuter usage

namespace engine {

Simulation::Simulation(cloe::Stack&& config, sol::state_view lua, const std::string& uuid)
    : config_(std::move(config))
    , lua_(std::move(lua))
    , logger_(cloe::logger::get("cloe"))
    , uuid_(uuid) {
  set_output_dir();
}

std::filesystem::path Simulation::get_output_filepath(const std::filesystem::path& filename) const {
  std::filesystem::path filepath;
  if (filename.is_absolute()) {
    filepath = filename;
  } else if (output_dir_) {
    filepath = *output_dir_ / filename;
  } else {
    throw cloe::ModelError{"cannot determine output path for '{}'", filename.native()};
  }

  return filepath;
}

void Simulation::set_output_dir() {
  if (config_.engine.output_path) {
    // For $registry to be of value, output_path (~= $id) here needs to be set.
    if (config_.engine.output_path->is_absolute()) {
      // If it's absolute, then registry_path doesn't matter.
      output_dir_ = *config_.engine.output_path;
    } else if (config_.engine.registry_path) {
      // Now, since output_dir is relative, we need the registry path.
      // We don't care here whether the registry is relative or not.
      output_dir_ = *config_.engine.registry_path / *config_.engine.output_path;
    }
  }
}

void Simulation::set_abort_handler(
    SimulationMachine& machine, SimulationContext& ctx, std::function<void()> hook) {
  abort_fn_ = [&, this]() {
    static size_t requests = 0;

    logger()->info("Signal caught.");
    if (hook) {
      hook();
    }
    requests += 1;

    if (ctx.progress.is_init_ended()) {
      if (!ctx.progress.is_exec_ended()) {
        logger()->info("Aborting running simulation.");
      }

      // Try to abort via the normal route first.
      if (requests == 1) {
        machine.abort();
        return;
      }
    } else {
      logger()->info("Aborting simulation configuration...");

      // Abort currently initializing model.
      cloe::Model* x = ctx.now_initializing;
      if (x != nullptr) {
        logger()->debug("Abort currently initializing model: {}", x->name());
        x->abort();
      }
    }

    // Tell everyone to abort.
    ctx.foreach_model([this](cloe::Model& y, const char* type) -> bool {
      try {
        logger()->debug("Abort {} {}", type, y.name());
        y.abort();
      } catch (std::exception& e) {
        logger()->error("Aborting {} {} failed: {}", type, y.name(), e.what());
      }
      return true;
    });
  };
}

SimulationResult Simulation::run() {
  auto machine = SimulationMachine();
  auto ctx = SimulationContext(config_, lua_.lua_state());
  auto errors = std::vector<std::string>();
  set_abort_handler(machine, ctx, [&errors]() {
    errors.emplace_back("user sent abort signal (e.g. with Ctrl+C)");
  });

  try {
    ctx.uuid = uuid_;
    ctx.report_progress = report_progress_;

    // Start the server if enabled
    if (config_.server.listen) {
      ctx.server->start();
    }
    // Stream data to the requested file
    if (config_.engine.output_file_data_stream) {
      auto filepath = get_output_filepath(*config_.engine.output_file_data_stream);
      if (is_writable(filepath)) {
        ctx.server->init_stream(filepath.native());
      }
    }

    // Run pre-connect hooks
    ctx.commander->set_enabled(config_.engine.security_enable_hooks);
    ctx.commander->run_all(config_.engine.hooks_pre_connect);
    ctx.commander->set_enabled(config_.engine.security_enable_commands);

    // Run the simulation
    cloe::luat_cloe_engine_state(lua_)["is_running"] = true;
    machine.run(ctx);
    cloe::luat_cloe_engine_state(lua_)["is_running"] = false;
  } catch (cloe::ConcludedError& e) {
    errors.emplace_back(e.what());
    ctx.outcome = SimulationOutcome::Aborted;
  } catch (std::exception& e) {
    errors.emplace_back(e.what());
    ctx.outcome = SimulationOutcome::Aborted;
  }

  try {
    // Run post-disconnect hooks
    ctx.commander->set_enabled(config_.engine.security_enable_hooks);
    ctx.commander->run_all(config_.engine.hooks_post_disconnect);
  } catch (cloe::ConcludedError& e) {
    // TODO(ben): ensure outcome is correctly saved
    errors.emplace_back(e.what());
  }

  // Wait for any running children to terminate.
  // (We could provide an option to time-out; this would involve using wait_for
  // instead of wait.)
  ctx.commander->wait_all();
  reset_abort_handler();

  auto result = ctx.result.value_or(SimulationResult{});
  result.outcome = ctx.outcome.value_or(SimulationOutcome::Aborted);
  assert(result.errors.empty()); // Not currently used in simulation.
  result.errors = errors;
  return result;
}

SimulationProbe Simulation::probe() {
  auto machine = SimulationMachine();
  auto ctx = SimulationContext(config_, lua_.lua_state());
  auto errors = std::vector<std::string>();
  set_abort_handler(machine, ctx, [&errors]() {
    errors.emplace_back("user sent abort signal (e.g. with Ctrl+C)");
  });

  try {
    ctx.uuid = uuid_;
    ctx.report_progress = report_progress_;

    // We deviate from run() by only doing the minimal amount of work here.
    // In particular:
    // - No server
    // - No commands / triggers
    // - No streaming file output
    // - Run pre-connect hooks only
    ctx.commander->set_enabled(config_.engine.security_enable_hooks);
    ctx.commander->run_all(config_.engine.hooks_pre_connect);

    ctx.probe_simulation = true;
    machine.run(ctx);
  } catch (cloe::ConcludedError& e) {
    errors.emplace_back(e.what());
    ctx.outcome = SimulationOutcome::Aborted;
  } catch (std::exception& e) {
    errors.emplace_back(e.what());
    ctx.outcome = SimulationOutcome::Aborted;
  }

  try {
    // Run post-disconnect hooks
    ctx.commander->set_enabled(config_.engine.security_enable_hooks);
    ctx.commander->run_all(config_.engine.hooks_post_disconnect);
  } catch (cloe::ConcludedError& e) {
    // TODO(ben): ensure outcome is correctly saved
    errors.emplace_back(e.what());
  }

  auto result = ctx.probe.value_or(SimulationProbe{});
  result.outcome = ctx.outcome.value_or(SimulationOutcome::Aborted);
  assert(result.errors.empty()); // Not currently used in simulation.
  result.errors = errors;
  return result;
}

size_t Simulation::write_output(const SimulationResult& r) const {
  if (output_dir_) {
    logger()->debug("Using output path: {}", output_dir_->native());
  }

  size_t files_written = 0;
  auto write_file = [&](auto filename, const cloe::Json& output) {
    if (!filename) {
      return;
    }

    std::filesystem::path filepath = get_output_filepath(*filename);
    if (write_output_file(filepath, output)) {
      files_written++;
    }
  };

  write_file(config_.engine.output_file_result, r);
  write_file(config_.engine.output_file_config, config_);
  write_file(config_.engine.output_file_triggers, r.triggers);
  // write_file(config_.engine.output_file_signals, .signals);
  // write_file(config_.engine.output_file_signals_autocompletion, r.signals_autocompletion);
  logger()->info("Wrote {} output files.", files_written);

  return files_written;
}

bool Simulation::write_output_file(const std::filesystem::path& filepath,
                                   const cloe::Json& j) const {
  if (!is_writable(filepath)) {
    return false;
  }
  auto native = filepath.native();
  std::ofstream ofs(native);
  if (ofs.fail()) {
    // throw error?
    logger()->error("Error opening file for writing: {}", native);
    return false;
  }
  logger()->debug("Writing file: {}", native);
  ofs << j.dump(2) << "\n";
  return true;
}

bool Simulation::is_writable(const std::filesystem::path& filepath) const {
  // Make sure we're not clobbering anything if we shouldn't.
  auto native = filepath.native();
  if (std::filesystem::exists(filepath)) {
    if (!config_.engine.output_clobber_files) {
      logger()->error("Will not clobber file: {}", native);
      return false;
    }
    if (!std::filesystem::is_regular_file(filepath)) {
      logger()->error("Cannot clobber non-regular file: {}", native);
      return false;
    }
  }

  // Make sure the directory exists.
  auto dirpath = filepath.parent_path();
  if (!std::filesystem::is_directory(dirpath)) {
    bool ok = std::filesystem::create_directories(dirpath);
    if (!ok) {
      logger()->error("Error creating leading directories: {}", dirpath.native());
      return false;
    }
  }

  return true;
}

// This is likely to be called when the user sends a signal that is caught
// by the signal handler. Because of the way connection handling is carried
// out, there is more than one thread in execution at this point. This makes
// doing the right thing extremely difficult.
//
// We don't know where we are in the simulation, so we will simply go through
// all models and tell them to abort.
void Simulation::signal_abort() {
  if (abort_fn_) {
    abort_fn_();
  }
}

}  // namespace engine
