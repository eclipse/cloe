/*
 * Copyright 2020 Robert Bosch GmbH
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

#include <cstdint>  // for uint64_t
#include <fstream>  // for ofstream
#include <future>   // for future<>, async
#include <sstream>  // for stringstream
#include <string>   // for string
#include <thread>   // for sleep_for

#include <boost/filesystem.hpp>  // for is_directory, is_regular_file, ...

#include <cloe/controller.hpp>                // for Controller
#include <cloe/core/abort.hpp>                // for AsyncAbort
#include <cloe/data_broker.hpp>               // for DataBroker
#include <cloe/registrar.hpp>                 // for DirectCallback
#include <cloe/simulator.hpp>                 // for Simulator
#include <cloe/trigger/example_actions.hpp>   // for CommandFactory, BundleFactory, ...
#include <cloe/trigger/set_action.hpp>        // for DEFINE_SET_STATE_ACTION, SetDataActionFactory
#include <cloe/utility/resource_handler.hpp>  // for INCLUDE_RESOURCE, RESOURCE_HANDLER
#include <cloe/vehicle.hpp>                   // for Vehicle
#include <fable/utility.hpp>                  // for pretty_print
#include <fable/utility/sol.hpp>              // for sol::object to_json

#include "coordinator.hpp"            // for register_usertype_coordinator
#include "lua_action.hpp"             // for LuaAction,
#include "lua_api.hpp"                // for to_json(json, sol::object)
#include "simulation_context.hpp"     // for SimulationContext
#include "utility/command.hpp"        // for CommandFactory
#include "utility/state_machine.hpp"  // for State, StateMachine
#include "utility/time_event.hpp"     // for TimeCallback, NextCallback, NextEvent, TimeEvent

// PROJECT_SOURCE_DIR is normally exported by CMake during build, but it's not
// available for the linters, so we define a dummy value here for that case.
#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR ""
#endif

INCLUDE_RESOURCE(index_html, PROJECT_SOURCE_DIR "/webui/index.html");
INCLUDE_RESOURCE(favicon, PROJECT_SOURCE_DIR "/webui/cloe_16x16.png");
INCLUDE_RESOURCE(cloe_logo, PROJECT_SOURCE_DIR "/webui/cloe.svg");
INCLUDE_RESOURCE(bootstrap_css, PROJECT_SOURCE_DIR "/webui/bootstrap.min.css");

namespace engine {

class SimulationMachine
    : private StateMachine<State<SimulationMachine, SimulationContext>, SimulationContext> {
  using SimulationState = State<SimulationMachine, SimulationContext>;

 public:
  SimulationMachine() {
    register_states({
        new Connect{this},
        new Start{this},
        new StepBegin{this},
        new StepSimulators{this},
        new StepControllers{this},
        new StepEnd{this},
        new Pause{this},
        new Resume{this},
        new Success{this},
        new Fail{this},
        new Abort{this},
        new Stop{this},
        new Reset{this},
        new KeepAlive{this},
        new Disconnect{this},
    });
  }

  void run(SimulationContext& ctx) { run_machine(CONNECT, ctx); }

  void run_machine(StateId initial, SimulationContext& ctx) {
    StateId id = initial;
    while (id != nullptr) {
      try {
        // Handle interrupts that have been inserted via push_interrupt.
        // Only one interrupt is stored.
        boost::optional<StateId> interrupt;
        while ((interrupt = pop_interrupt())) {
          id = handle_interrupt(id, *interrupt, ctx);
        }

        if (ctx.config.engine.watchdog_mode == cloe::WatchdogMode::Off) {
          // Run state in this thread synchronously.
          id = run_state(id, ctx);
          continue;
        }

        // Run state in a separate thread asynchronously and abort if
        // watchdog_timeout is exceeded.
        //
        // See configuration: stack.hpp
        // See documentation: doc/reference/watchdog.rst
        std::chrono::milliseconds timeout = ctx.config.engine.watchdog_default_timeout;
        if (ctx.config.engine.watchdog_state_timeouts.count(id)) {
          auto maybe = ctx.config.engine.watchdog_state_timeouts[id];
          if (maybe) {
            timeout = *maybe;
          }
        }
        auto interval = timeout.count() > 0 ? timeout : ctx.config.engine.polling_interval;

        // Launch state
        std::future<StateId> f =
            std::async(std::launch::async, [this, id, &ctx]() { return run_state(id, ctx); });

        std::future_status status;
        for (;;) {
          status = f.wait_for(interval);
          if (status == std::future_status::ready) {
            id = f.get();
            break;
          } else if (status == std::future_status::deferred) {
            if (timeout.count() > 0) {
              logger()->warn("Watchdog waiting on deferred execution.");
            }
          } else if (status == std::future_status::timeout) {
            if (timeout.count() > 0) {
              logger()->critical("Watchdog timeout of {} ms exceeded for state: {}",
                                 timeout.count(), id);

              if (ctx.config.engine.watchdog_mode == cloe::WatchdogMode::Abort) {
                logger()->critical("Aborting simulation... this might take a while...");
                this->push_interrupt(ABORT);
                break;
              } else if (ctx.config.engine.watchdog_mode == cloe::WatchdogMode::Kill) {
                logger()->critical("Killing program... this is going to be messy...");
                std::abort();
              }
            }
          }
        }
      } catch (cloe::AsyncAbort&) {
        this->push_interrupt(ABORT);
      } catch (cloe::ModelReset& e) {
        logger()->error("Unhandled reset request in {} state: {}", id, e.what());
        this->push_interrupt(RESET);
      } catch (cloe::ModelStop& e) {
        logger()->error("Unhandled stop request in {} state: {}", id, e.what());
        this->push_interrupt(STOP);
      } catch (cloe::ModelAbort& e) {
        logger()->error("Unhandled abort request in {} state: {}", id, e.what());
        this->push_interrupt(ABORT);
      } catch (cloe::ModelError& e) {
        logger()->error("Unhandled model error in {} state: {}", id, e.what());
        this->push_interrupt(ABORT);
      } catch (std::exception& e) {
        logger()->critical("Fatal error in {} state: {}", id, e.what());
        throw;
      }
    }
  }

  // Asynchronous Actions:
  void pause() { this->push_interrupt(PAUSE); }
  void resume() { this->push_interrupt(RESUME); }
  void stop() { this->push_interrupt(STOP); }
  void succeed() { this->push_interrupt(SUCCESS); }
  void fail() { this->push_interrupt(FAIL); }
  void reset() { this->push_interrupt(RESET); }
  void abort() { this->push_interrupt(ABORT); }

  StateId handle_interrupt(StateId nominal, StateId interrupt, SimulationContext& ctx) override {
    logger()->debug("Handle interrupt: {}", interrupt);
    // We don't necessarily actually go directly to each desired state. The
    // states PAUSE and RESUME are prime examples; they should be entered and
    // exited from at pre-defined points.
    if (interrupt == PAUSE) {
      ctx.pause_execution = true;
    } else if (interrupt == RESUME) {
      ctx.pause_execution = false;
    } else {
      // All other interrupts will lead directly to the end of the
      // simulation.
      return this->run_state(interrupt, ctx);
    }
    return nominal;
  }

  friend void to_json(cloe::Json& j, const SimulationMachine& m) {
    j = cloe::Json{
        {"states", m.states()},
    };
  }

#define DEFINE_STATE(Id, S) DEFINE_STATE_STRUCT(SimulationMachine, SimulationContext, Id, S)
 public:
  DEFINE_STATE(CONNECT, Connect);
  DEFINE_STATE(START, Start);
  DEFINE_STATE(STEP_BEGIN, StepBegin);
  DEFINE_STATE(STEP_SIMULATORS, StepSimulators);
  DEFINE_STATE(STEP_CONTROLLERS, StepControllers);
  DEFINE_STATE(STEP_END, StepEnd);
  DEFINE_STATE(PAUSE, Pause);
  DEFINE_STATE(RESUME, Resume);
  DEFINE_STATE(SUCCESS, Success);
  DEFINE_STATE(FAIL, Fail);
  DEFINE_STATE(ABORT, Abort);
  DEFINE_STATE(STOP, Stop);
  DEFINE_STATE(RESET, Reset);
  DEFINE_STATE(KEEP_ALIVE, KeepAlive);
  DEFINE_STATE(DISCONNECT, Disconnect);
#undef DEFINE_STATE
};

namespace actions {

// clang-format off
DEFINE_SET_STATE_ACTION(Pause, "pause", "pause simulation", SimulationMachine, { ptr_->pause(); })
DEFINE_SET_STATE_ACTION(Resume, "resume", "resume paused simulation", SimulationMachine, { ptr_->resume(); })
DEFINE_SET_STATE_ACTION(Stop, "stop", "stop simulation with neither success nor failure", SimulationMachine, { ptr_->stop(); })
DEFINE_SET_STATE_ACTION(Succeed, "succeed", "stop simulation with success", SimulationMachine, { ptr_->succeed(); })
DEFINE_SET_STATE_ACTION(Fail, "fail", "stop simulation with failure", SimulationMachine, { ptr_->fail(); })
DEFINE_SET_STATE_ACTION(Reset, "reset", "attempt to reset simulation", SimulationMachine, { ptr_->reset(); })
DEFINE_SET_STATE_ACTION(KeepAlive, "keep_alive", "keep simulation alive after termination", SimulationContext, { ptr_->config.engine.keep_alive = true; })
DEFINE_SET_STATE_ACTION(ResetStatistics, "reset_statistics", "reset simulation statistics", SimulationStatistics, { ptr_->reset(); })

DEFINE_SET_DATA_ACTION(RealtimeFactor, "realtime_factor", "modify the simulation speed", SimulationSync, "factor", double,
                        {
                         logger()->info("Setting target simulation speed: {}", value_);
                         ptr_->set_realtime_factor(value_);
                        })

// clang-format on

}  // namespace actions

std::string enumerate_simulator_vehicles(const cloe::Simulator& s) {
  std::stringstream buffer;
  auto n = s.num_vehicles();
  for (size_t i = 0; i < n; i++) {
    auto v = s.get_vehicle(i);
    buffer << fmt::format("{}: {}\n", i, v->name());
  }
  return buffer.str();
}

void handle_cloe_error(cloe::Logger logger, const cloe::Error& e) {
  if (e.has_explanation()) {
    logger->error("Note:\n{}", fable::indent_string(e.explanation(), "    "));
  }
}

// CONNECT ------------------------------------------------------------------------------------- //

StateId SimulationMachine::Connect::impl(SimulationContext& ctx) {
  logger()->info("Initializing simulation...");
  assert(ctx.config.is_valid());

  // 1. Initialize progress tracking
  ctx.progress.init_begin(6);
  auto update_progress = [&ctx](const char* str) {
    ctx.progress.init(str);
    ctx.server->refresh_buffer();
  };

  {  // 2. Initialize loggers
    update_progress("logging");

    for (const auto& c : ctx.config.logging) {
      c.apply();
    }
  }

  {  // 3. Initialize Lua
    auto cloe_tbl = sol::object(ctx.lua["cloe"]).as<sol::table>();
    register_usertype_coordinator(cloe_tbl, ctx.sync);
    cloe_tbl["state"]["scheduler"] = std::ref(*ctx.coordinator);
  }

  {  // 4. Enroll endpoints and triggers for the server
    update_progress("server");

    auto rp = ctx.simulation_registrar();
    cloe::Registrar& r = *rp;

    // HTML endpoints:
    r.register_static_handler("/", RESOURCE_HANDLER(index_html, cloe::ContentType::HTML));
    r.register_static_handler("/index.html", cloe::handler::Redirect("/"));
    r.register_static_handler("/cloe_16x16.png", RESOURCE_HANDLER(favicon, cloe::ContentType::PNG));
    r.register_static_handler("/cloe.svg", RESOURCE_HANDLER(cloe_logo, cloe::ContentType::SVG));
    r.register_static_handler("/bootstrap.css",
                              RESOURCE_HANDLER(bootstrap_css, cloe::ContentType::CSS));

    // API endpoints:
    r.register_api_handler("/uuid", cloe::HandlerType::STATIC, cloe::handler::StaticJson(ctx.uuid));
    r.register_api_handler("/version", cloe::HandlerType::STATIC,
                           cloe::handler::StaticJson(ctx.version()));
    r.register_api_handler("/progress", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SimulationProgress>(&ctx.progress));
    r.register_api_handler(
        "/configuration", cloe::HandlerType::DYNAMIC,
        [&ctx](const cloe::Request& q, cloe::Response& r) {
          std::string type = "active";
          auto m = q.query_map();
          if (m.count("type")) {
            type = m.at("type");
          }

          if (type == "active") {
            r.write(ctx.config.active_config());
          } else if (type == "input") {
            r.write(ctx.config.input_config());
          } else {
            r.bad_request(cloe::Json{
                {"error", "invalid type value"},
                {"fields", {{"type", "configuration output type, one of: active, input"}}},
            });
          }
        });
    r.register_api_handler("/simulation", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SimulationSync>(&ctx.sync));
    r.register_api_handler("/statistics", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SimulationStatistics>(&ctx.statistics));
    r.register_api_handler("/plugins", cloe::HandlerType::STATIC,
                           cloe::handler::StaticJson(ctx.plugin_ids()));

    // Coordinator & Server
    ctx.server->enroll(r);
    ctx.coordinator->enroll(r);

    // Events:
    ctx.callback_loop = r.register_event<events::LoopFactory>();
    ctx.callback_start = r.register_event<events::StartFactory>();
    ctx.callback_stop = r.register_event<events::StopFactory>();
    ctx.callback_success = r.register_event<events::SuccessFactory>();
    ctx.callback_failure = r.register_event<events::FailureFactory>();
    ctx.callback_reset = r.register_event<events::ResetFactory>();
    ctx.callback_pause = r.register_event<events::PauseFactory>();
    ctx.callback_resume = r.register_event<events::ResumeFactory>();
    ctx.callback_time = std::make_shared<events::TimeCallback>(
        logger(), [this, &ctx](const cloe::Trigger& t, cloe::Duration when) {
          static const std::vector<std::string> eta_names{"stop", "succeed", "fail", "reset"};
          auto name = t.action().name();
          for (std::string x : eta_names) {
            // Take possible namespacing of simulation actions into account.
            if (ctx.config.simulation.name) {
              x = *ctx.config.simulation.name + "/" + x;
            }
            if (name == x) {
              // We are only interested in the earliest stop action.
              if (ctx.sync.eta() == cloe::Duration(0) || when < ctx.sync.eta()) {
                logger()->info("Set simulation ETA to {}s", cloe::Seconds{when}.count());
                ctx.sync.set_eta(when);
                ctx.progress.execution_eta = when;
              }
            }
          }
        });
    r.register_event(std::make_unique<events::TimeFactory>(), ctx.callback_time);
    r.register_event(std::make_unique<events::NextFactory>(),
                     std::make_shared<events::NextCallback>(ctx.callback_time));

    // Actions:
    r.register_action<actions::PauseFactory>(this->state_machine());
    r.register_action<actions::ResumeFactory>(this->state_machine());
    r.register_action<actions::StopFactory>(this->state_machine());
    r.register_action<actions::ResetFactory>(this->state_machine());
    r.register_action<actions::FailFactory>(this->state_machine());
    r.register_action<actions::SucceedFactory>(this->state_machine());
    r.register_action<actions::KeepAliveFactory>(&ctx);
    r.register_action<actions::RealtimeFactorFactory>(&ctx.sync);
    r.register_action<actions::ResetStatisticsFactory>(&ctx.statistics);
    r.register_action<actions::CommandFactory>(ctx.commander.get());
    r.register_action<actions::LuaFactory>(ctx.lua);

    // From: cloe/trigger/example_actions.hpp
    auto tr = ctx.coordinator->trigger_registrar(cloe::Source::TRIGGER);
    r.register_action<cloe::actions::BundleFactory>(tr);
    r.register_action<cloe::actions::InsertFactory>(tr);
    r.register_action<cloe::actions::LogFactory>();
    r.register_action<cloe::actions::PushReleaseFactory>(tr);
  }

  {  // 5. Initialize simulators
    update_progress("simulators");

    /**
     * Return a new Simulator given configuration c.
     */
    auto new_simulator = [&ctx](const cloe::SimulatorConf& c) -> std::unique_ptr<cloe::Simulator> {
      auto f = c.factory->clone();
      auto name = c.name.value_or(c.binding);
      for (auto d : ctx.config.get_simulator_defaults(name, f->name())) {
        f->from_conf(d.args);
      }
      auto x = f->make(c.args);
      ctx.now_initializing = x.get();

      // Configure simulator:
      auto r = ctx.registrar->with_trigger_prefix(name)->with_api_prefix(
          std::string("/simulators/") + name);
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    // Create and configure all simulators:
    for (const auto& c : ctx.config.simulators) {
      auto name = c.name.value_or(c.binding);
      assert(ctx.simulators.count(name) == 0);
      logger()->info("Configure simulator {}", name);

      try {
        ctx.simulators[name] = new_simulator(c);
      } catch (cloe::ModelError& e) {
        logger()->critical("Error configuring simulator {}: {}", name, e.what());
        return ABORT;
      }
    }

    auto r = ctx.simulation_registrar();
    r->register_api_handler("/simulators", cloe::HandlerType::STATIC,
                            cloe::handler::StaticJson(ctx.simulator_ids()));
  }

  {  // 6. Initialize vehicles
    update_progress("vehicles");

    /**
     * Return a new Component given vehicle v and configuration c.
     */
    auto new_component = [&ctx](cloe::Vehicle& v,
                                const cloe::ComponentConf& c) -> std::shared_ptr<cloe::Component> {
      // Create a copy of the component factory prototype and initialize it with the default stack arguments.
      auto f = c.factory->clone();
      auto name = c.name.value_or(c.binding);
      for (auto d : ctx.config.get_component_defaults(name, f->name())) {
        f->from_conf(d.args);
      }
      // Get input components, if applicable.
      std::vector<std::shared_ptr<cloe::Component>> from;
      for (const auto& from_comp_name : c.from) {
        if (!v.has(from_comp_name)) {
          return nullptr;
        }
        from.push_back(v.get<cloe::Component>(from_comp_name));
      }
      // Create the new component.
      auto x = f->make(c.args, std::move(from));
      ctx.now_initializing = x.get();

      // Configure component:
      auto r = ctx.registrar->with_trigger_prefix(name)->with_api_prefix(
          std::string("/components/") + name);
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    /**
     * Return a new Vehicle given configuration c.
     */
    auto new_vehicle = [&](const cloe::VehicleConf& c) -> std::shared_ptr<cloe::Vehicle> {
      static uint64_t gid = 1024;

      // Fetch vehicle prototype.
      std::shared_ptr<cloe::Vehicle> v;
      if (c.is_from_simulator()) {
        auto& s = ctx.simulators.at(c.from_sim.simulator);
        if (c.from_sim.is_by_name()) {
          v = s->get_vehicle(c.from_sim.index_str);
          if (!v) {
            throw cloe::ModelError("simulator {} has no vehicle by name {}", c.from_sim.simulator,
                                   c.from_sim.index_str)
                .explanation("Simulator {} has following vehicles:\n{}", c.from_sim.simulator,
                             enumerate_simulator_vehicles(*s));
          }
        } else {
          v = s->get_vehicle(c.from_sim.index_num);
          if (!v) {
            throw cloe::ModelError("simulator {} has no vehicle at index {}", c.from_sim.simulator,
                                   c.from_sim.index_num)
                .explanation("Simulator {} has following vehicles:\n{}", c.from_sim.simulator,
                             enumerate_simulator_vehicles(*s));
          }
        }
      } else {
        if (ctx.vehicles.count(c.from_veh)) {
          v = ctx.vehicles.at(c.from_veh);
        } else {
          // This vehicle depends on another that hasn't been create yet.
          return nullptr;
        }
      }

      // Create vehicle from prototype and configure the components.
      logger()->info("Configure vehicle {}", c.name);
      auto x = v->clone(++gid, c.name);
      ctx.now_initializing = x.get();

      std::set<std::string> configured;
      size_t n = c.components.size();
      while (configured.size() != n) {
        // Keep trying to create components until all have been created.
        // This is a poor-man's version of dependency resolution and has O(n^2)
        // complexity, which is acceptable given that the expected number of
        // components is usually less than 100.
        size_t m = configured.size();
        for (const auto& kv : c.components) {
          if (configured.count(kv.first)) {
            // This component has already been configured.
            continue;
          }

          auto k = new_component(*x, kv.second);
          if (k) {
            x->set_component(kv.first, std::move(k));
            configured.insert(kv.first);
          }
        }

        // Check that we are making progress.
        if (configured.size() == m) {
          // We have configured.size() != n and has not grown since going
          // through all Component configs. This means that we have some unresolved
          // dependencies. Find out which and abort.
          for (const auto& kv : c.components) {
            if (configured.count(kv.first)) {
              continue;
            }

            // We now have a component that has not been configured, and this
            // can only be the case if the dependency is not found.
            assert(kv.second.from.size() > 0);
            for (const auto& from_comp_name : kv.second.from) {
              if (x->has(from_comp_name)) {
                continue;
              }
              throw cloe::ModelError{
                  "cannot configure component '{}': cannot resolve dependency '{}'",
                  kv.first,
                  from_comp_name,
              };
            }
          }
        }
      }

      // Configure vehicle:
      auto r = ctx.registrar->with_trigger_prefix(c.name)->with_api_prefix(
          std::string("/vehicles/") + c.name);
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    // Create and configure all vehicles:
    size_t n = ctx.config.vehicles.size();
    while (ctx.vehicles.size() != n) {
      // Keep trying to create vehicles until all have been created.
      // This is a poor-man's version of dependency resolution and has O(n^2)
      // complexity, which is acceptable given that the expected number of
      // vehicles is almost always less than 10.
      size_t m = ctx.vehicles.size();
      for (const auto& c : ctx.config.vehicles) {
        if (ctx.vehicles.count(c.name)) {
          // This vehicle has already been configured.
          continue;
        }

        std::shared_ptr<cloe::Vehicle> v;
        try {
          v = new_vehicle(c);
        } catch (cloe::ModelError& e) {
          logger()->critical("Error configuring vehicle {}: {}", c.name, e.what());
          handle_cloe_error(logger(), e);
          return ABORT;
        }

        if (v) {
          ctx.vehicles[c.name] = std::move(v);
        }
      }

      // Check that we are making progress.
      if (ctx.vehicles.size() == m) {
        // We have ctx.vehicles.size() != n and has not grown since going
        // through all Vehicle configs. This means that we have some unresolved
        // dependencies. Find out which and abort.
        for (const auto& c : ctx.config.vehicles) {
          if (ctx.vehicles.count(c.name)) {
            continue;
          }

          // We now have a vehicle that has not been configured, and this can
          // only be the case if a vehicle dependency is not found.
          assert(c.is_from_vehicle());
          throw cloe::ModelError{
              "cannot configure vehicle '{}': cannot resolve dependency '{}'",
              c.name,
              c.from_veh,
          };
        }
      }
    }

    auto r = ctx.simulation_registrar();
    r->register_api_handler("/vehicles", cloe::HandlerType::STATIC,
                            cloe::handler::StaticJson(ctx.vehicle_ids()));
  }

  {  // 7. Initialize controllers
    update_progress("controllers");

    /**
     * Return a new Controller given configuration c.
     */
    auto new_controller =
        [&ctx](const cloe::ControllerConf& c) -> std::unique_ptr<cloe::Controller> {
      auto f = c.factory->clone();
      auto name = c.name.value_or(c.binding);
      for (auto d : ctx.config.get_controller_defaults(name, f->name())) {
        f->from_conf(d.args);
      }
      auto x = f->make(c.args);
      ctx.now_initializing = x.get();

      // Configure
      auto r = ctx.registrar->with_trigger_prefix(name)->with_api_prefix(
          std::string("/controllers/") + name);
      x->set_vehicle(ctx.vehicles.at(c.vehicle));
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    // Create and configure all controllers:
    for (const auto& c : ctx.config.controllers) {
      auto name = c.name.value_or(c.binding);
      assert(ctx.controllers.count(name) == 0);
      logger()->info("Configure controller {}", name);
      try {
        ctx.controllers[name] = new_controller(c);
      } catch (cloe::ModelError& e) {
        logger()->critical("Error configuring controller {}: {}", name, e.what());
        return ABORT;
      }
    }

    auto r = ctx.simulation_registrar();
    r->register_api_handler("/controllers", cloe::HandlerType::STATIC,
                            cloe::handler::StaticJson(ctx.controller_ids()));
  }

  ctx.progress.init_end();
  ctx.server->refresh_buffer_start_stream();
  logger()->info("Simulation initialization complete.");
  return START;
}

// START --------------------------------------------------------------------------------------- //

size_t insert_triggers_from_config(SimulationContext& ctx) {
  auto r = ctx.coordinator->trigger_registrar(cloe::Source::FILESYSTEM);
  size_t count = 0;
  for (const auto& c : ctx.config.triggers) {
    if (!ctx.config.engine.triggers_ignore_source && source_is_transient(c.source)) {
      continue;
    }
    try {
      r->insert_trigger(c.conf());
      count++;
    } catch (cloe::SchemaError& e) {
      ctx.logger()->error("Error inserting trigger: {}", e.what());
      std::stringstream s;
      fable::pretty_print(e, s);
      ctx.logger()->error("> Message:\n    {}", s.str());
      throw cloe::ConcludedError(e);
    } catch (cloe::TriggerError& e) {
      ctx.logger()->error("Error inserting trigger ({}): {}", e.what(), c.to_json().dump());
      throw cloe::ConcludedError(e);
    }
  }
  return count;
}

StateId SimulationMachine::Start::impl(SimulationContext& ctx) {
  logger()->info("Starting simulation...");

  // Begin execution progress
  ctx.progress.exec_begin();

  {
    // Bind lua state_view to databroker
    auto* dbPtr = ctx.coordinator->data_broker();
    if (!dbPtr) {
      throw std::logic_error("Coordinator did not provide a DataBroker instance");
    }
    auto& db = *dbPtr;
    // Alias signals via lua
    {
      bool aliasing_failure = false;
      // Read cloe.alias_signals
      sol::object value = ctx.lua["cloe"]["alias_signals"];
      auto type = value.get_type();
      switch (type) {
        // cloe.alias_signals: expected is a list (i.e. table) of 2-tuple each strings
        case sol::type::table: {
          sol::table alias_signals = value.as<sol::table>();
          auto tbl_size = std::distance(alias_signals.begin(), alias_signals.end());
          //for (auto& kv : alias_signals)
          for (int i = 0; i < tbl_size; i++) {
            //sol::object value = kv.second;
            sol::object value = ctx.lua["cloe"]["alias_signals"][i + 1];
            sol::type type = value.get_type();
            switch (type) {
              // cloe.alias_signals[i]: expected is a 2-tuple (i.e. table) each strings
              case sol::type::table: {
                sol::table alias_tuple = value.as<sol::table>();
                auto tbl_size = std::distance(alias_tuple.begin(), alias_tuple.end());
                if (tbl_size != 2) {
                  // clang-format off
                  logger()->error(
                      "One or more entries in 'cloe.alias_signals' does not consist out of a 2-tuple. "
                      "Expected are entries in this format { \"regex\" , \"short-name\" }"
                      );
                  // clang-format on
                  aliasing_failure = true;
                  continue;
                }

                sol::object value;
                sol::type type;
                std::string old_name;
                std::string alias_name;
                value = alias_tuple[1];
                type = value.get_type();
                if (sol::type::string != type) {
                  // clang-format off
                  logger()->error(
                      "One or more parts in a tuple in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                      "Expected are entries in this format { \"regex\" , \"short-name\" }",
                      static_cast<int>(type));
                  // clang-format on
                  aliasing_failure = true;
                } else {
                  old_name = value.as<std::string>();
                }

                value = alias_tuple[2];
                type = value.get_type();
                if (sol::type::string != type) {
                  // clang-format off
                  logger()->error(
                      "One or more parts in a tuple in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                      "Expected are entries in this format { \"regex\" , \"short-name\" }",
                      static_cast<int>(type));
                  // clang-format on
                  aliasing_failure = true;
                } else {
                  alias_name = value.as<std::string>();
                }
                try {
                  db.alias(old_name, alias_name);
                  // clang-format off
                  logger()->info(
                      "Aliasing signal '{}' as '{}'.",
                      old_name, alias_name);
                  // clang-format on
                } catch (const std::logic_error& ex) {
                  // clang-format off
                  logger()->error(
                      "Aliasing signal specifier '{}' as '{}' failed with this error: {}",
                      old_name, alias_name, ex.what());
                  // clang-format on
                  aliasing_failure = true;
                } catch (...) {
                  // clang-format off
                  logger()->error(
                      "Aliasing signal specifier '{}' as '{}' failed.",
                      old_name, alias_name);
                  // clang-format on
                  aliasing_failure = true;
                }
              } break;
              // cloe.alias_signals[i]: is not a table
              default: {
                // clang-format off
                logger()->error(
                    "One or more entries in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                    "Expected are entries in this format { \"regex\" , \"short-name\" }",
                    static_cast<int>(type));
                // clang-format on
                aliasing_failure = true;
              } break;
            }
          }

        } break;
        case sol::type::none:
        case sol::type::lua_nil: {
          // not defined -> nop
        } break;
        default: {
          // clang-format off
          logger()->error(
              "Expected symbol 'cloe.alias_signals' has unexpected datatype '{}'. "
              "Expected is a list of 2-tuples in this format { \"regex\" , \"short-name\" }",
              static_cast<int>(type));
          // clang-format on
          aliasing_failure = true;
        } break;
      }
      if (aliasing_failure) {
        throw cloe::ModelError("Aliasing signals failed with above error. Aborting.");
      }
    }
    // Inject requested signals into lua
    {
      auto& signals = db.signals();
      bool binding_failure = false;
      // Read cloe.require_signals
      sol::object value = ctx.lua["cloe"]["require_signals"];
      auto type = value.get_type();
      switch (type) {
        // cloe.require_signals expected is a list (i.e. table) of strings
        case sol::type::table: {
          sol::table require_signals = value.as<sol::table>();
          auto tbl_size = std::distance(require_signals.begin(), require_signals.end());

          for (int i = 0; i < tbl_size; i++) {
            sol::object value = ctx.lua["cloe"]["require_signals"][i + 1];

            sol::type type = value.get_type();
            if (type != sol::type::string) {
              // clang-format off
              logger()->warn(
                  "One entry of cloe.require_signals has a wrong data type: '{}'. "
                  "Expected is a list of strings.",
                  static_cast<int>(type));
              // clang-format on
              binding_failure = true;
              continue;
            }
            std::string signal_name = value.as<std::string>();

            // bind signal 'signal_name' to lua
            auto iter = db[signal_name];
            if (iter != signals.end()) {
              try {
                db.bind(signal_name, signal_name);
                // clang-format off
                  logger()->info(
                      "Binding signal '{}' as '{}'.",
                      signal_name, signal_name);
                // clang-format on
              } catch (const std::logic_error& ex) {
                // clang-format off
                logger()->error(
                    "Binding signal '{}' failed with error: {}",
                    signal_name, ex.what()
                    );
                // clang-format on
              }
            } else {
              // clang-format off
              logger()->warn(
                  "Requested signal '{}' does not exist in DataBroker.",
                  signal_name
                  );
              // clang-format on
              binding_failure = true;
            }
          }
        } break;
        case sol::type::none:
        case sol::type::lua_nil: {
          // clang-format off
          logger()->warn(
              "Expected symbol 'cloe.require_signals' appears to be undefined. "
              "Expected is a list of string."
              );
          // clang-format on
        } break;
        default: {
          // clang-format off
          logger()->error(
              "Expected symbol 'cloe.require_signals' has unexpected datatype '{}'. "
              "Expected is a list of string.",
              static_cast<int>(type));
          // clang-format on
          binding_failure = true;
        } break;
      }
      if (binding_failure) {
        throw cloe::ModelError("Binding signals to Lua failed with above error. Aborting.");
      }
    }
  }

  // Process initial trigger list
  insert_triggers_from_config(ctx);
  ctx.coordinator->process_pending_lua_triggers(ctx.sync);
  ctx.coordinator->process(ctx.sync);
  ctx.callback_start->trigger(ctx.sync);

  // Process initial context
  ctx.foreach_model([this, &ctx](cloe::Model& m, const char* type) {
    logger()->trace("Start {} {}", type, m.name());
    m.start(ctx.sync);
    return true;  // next model
  });
  ctx.sync.increment_step();

  // We can pause at the start of execution too.
  if (ctx.pause_execution) {
    return PAUSE;
  }

  return STEP_BEGIN;
}

// STEP_BEGIN ---------------------------------------------------------------------------------- //

StateId SimulationMachine::StepBegin::impl(SimulationContext& ctx) {
  ctx.cycle_duration.reset();
  timer::DurationTimer<cloe::Duration> t([&ctx](cloe::Duration d) {
    auto ms = std::chrono::duration_cast<cloe::Milliseconds>(d);
    ctx.statistics.engine_time_ms.push_back(ms.count());
  });

  logger()->trace("Step {:0>9}, Time {} ms", ctx.sync.step(),
                  std::chrono::duration_cast<cloe::Milliseconds>(ctx.sync.time()).count());

  // Update execution progress
  ctx.progress.exec_update(ctx.sync.time());
  if (ctx.report_progress && ctx.progress.exec_report()) {
    logger()->info("Execution progress: {}%",
                   static_cast<int>(ctx.progress.execution.percent() * 100.0));
  }

  // Refresh the double buffer
  //
  // Note: this line can easily break your time budget with the current server
  // implementation. If you need better performance, disable the server in the
  // stack file configuration:
  //
  //   {
  //     "version": "4",
  //     "server": {
  //       "listen": false
  //     }
  //   }
  //
  ctx.server->refresh_buffer();

  // Run cycle- and time-based triggers
  ctx.callback_loop->trigger(ctx.sync);
  ctx.callback_time->trigger(ctx.sync);

  // Determine whether to continue simulating or stop
  bool all_operational = ctx.foreach_model([this](const cloe::Model& m, const char* type) {
    if (!m.is_operational()) {
      logger()->info("The {} {} is no longer operational.", type, m.name());
      return false;  // abort loop
    }
    return true;  // next model
  });
  return (all_operational ? STEP_SIMULATORS : STOP);
}

// STEP_SIMULATORS ----------------------------------------------------------------------------- //

StateId SimulationMachine::StepSimulators::impl(SimulationContext& ctx) {
  auto guard = ctx.server->lock();

  timer::DurationTimer<cloe::Duration> t([&ctx](cloe::Duration d) {
    auto ms = std::chrono::duration_cast<cloe::Milliseconds>(d);
    ctx.statistics.simulator_time_ms.push_back(ms.count());
  });

  // Call the simulator bindings:
  ctx.foreach_simulator([&ctx](cloe::Simulator& simulator) {
    try {
      cloe::Duration sim_time = simulator.process(ctx.sync);
      if (!simulator.is_operational()) {
        throw cloe::ModelStop("simulator {} no longer operational", simulator.name());
      }
      if (sim_time != ctx.sync.time()) {
        throw cloe::ModelError(
            "simulator {} did not progress to required time: got {}ms, expected {}ms",
            simulator.name(), sim_time.count() / 1'000'000, ctx.sync.time().count() / 1'000'000);
      }
    } catch (cloe::ModelReset& e) {
      throw;
    } catch (cloe::ModelStop& e) {
      throw;
    } catch (cloe::ModelAbort& e) {
      throw;
    } catch (cloe::ModelError& e) {
      throw;
    } catch (...) {
      throw;
    }
    return true;
  });

  // Clear vehicle cache
  ctx.foreach_vehicle([this, &ctx](cloe::Vehicle& v) {
    auto t = v.process(ctx.sync);
    if (t < ctx.sync.time()) {
      logger()->error("Vehicle ({}, {}) not progressing; simulation compromised!", v.id(),
                      v.name());
    }
    return true;
  });

  return STEP_CONTROLLERS;
}

// STEP_CONTROLLERS ---------------------------------------------------------------------------- //

StateId SimulationMachine::StepControllers::impl(SimulationContext& ctx) {
  auto guard = ctx.server->lock();

  timer::DurationTimer<cloe::Duration> t([&ctx](cloe::Duration d) {
    auto ms = std::chrono::duration_cast<cloe::Milliseconds>(d);
    ctx.statistics.controller_time_ms.push_back(ms.count());
  });

  // We can only erase from ctx.controllers when we have access to the
  // iterator itself, otherwise we get undefined behavior. So we save
  // the names of the controllers we want to erase from the list.
  std::vector<std::string> controllers_to_erase;

  // Call each controller and handle any errors that might occur.
  ctx.foreach_controller([this, &ctx, &controllers_to_erase](cloe::Controller& ctrl) {
    if (!ctrl.has_vehicle()) {
      // Skip this controller
      return true;
    }

    // Keep calling the ctrl until it has caught up the current time.
    cloe::Duration ctrl_time;
    try {
      int64_t retries = 0;
      for (;;) {
        ctrl_time = ctrl.process(ctx.sync);

        // If we are underneath our target, sleep and try again.
        if (ctrl_time < ctx.sync.time()) {
          this->logger()->warn("Controller {} not progressing, now at {}", ctrl.name(),
                               cloe::to_string(ctrl_time));

          // If a controller is misbehaving, we might get stuck in a loop.
          // If this happens more than some random high number, then throw
          // an error.
          if (retries == ctx.config.simulation.controller_retry_limit) {
            throw cloe::ModelError{"controller not progressing to target time {}",
                                   cloe::to_string(ctx.sync.time())};
          }

          // Otherwise, sleep and try again.
          std::this_thread::sleep_for(ctx.config.simulation.controller_retry_sleep);
          ++retries;
        } else {
          ctx.statistics.controller_retries.push_back(static_cast<double>(retries));
          break;
        }
      }
    } catch (cloe::ModelReset& e) {
      this->logger()->error("Controller {} reset: {}", ctrl.name(), e.what());
      this->state_machine()->reset();
      return false;
    } catch (cloe::ModelStop& e) {
      this->logger()->error("Controller {} stop: {}", ctrl.name(), e.what());
      this->state_machine()->stop();
      return false;
    } catch (cloe::ModelAbort& e) {
      this->logger()->error("Controller {} abort: {}", ctrl.name(), e.what());
      this->state_machine()->abort();
      return false;
    } catch (cloe::Error& e) {
      this->logger()->error("Controller {} died: {}", ctrl.name(), e.what());
      if (e.has_explanation()) {
        this->logger()->error("Note:\n{}", e.explanation());
      }
      if (ctx.config.simulation.abort_on_controller_failure) {
        this->logger()->error("Aborting thanks to controller {}", ctrl.name());
        this->state_machine()->abort();
        return false;
      } else {
        this->logger()->warn("Continuing without controller {}", ctrl.name());
        ctrl.abort();
        ctrl.disconnect();
        controllers_to_erase.push_back(ctrl.name());
        return true;
      }
    } catch (...) {
      this->logger()->critical("Controller {} encountered a fatal error.", ctrl.name());
      throw;
    }

    // Write a notice if the controller is ahead of the simulation time.
    cloe::Duration ctrl_ahead = ctrl_time - ctx.sync.time();
    if (ctrl_ahead.count() > 0) {
      this->logger()->warn("Controller {} is ahead by {}", ctrl.name(),
                           cloe::to_string(ctrl_ahead));
    }

    // Continue with next controller.
    return true;
  });

  // Remove any controllers that we want to continue without.
  for (auto ctrl : controllers_to_erase) {
    ctx.controllers.erase(ctrl);
  }

  return STEP_END;
}

// STEP_END ------------------------------------------------------------------------------------ //

StateId SimulationMachine::StepEnd::impl(SimulationContext& ctx) {
  // Adjust sim time to wallclock according to realtime factor.
  cloe::Duration padding = cloe::Duration{0};
  cloe::Duration elapsed = ctx.cycle_duration.elapsed();
  {
    auto guard = ctx.server->lock();
    ctx.sync.set_cycle_time(elapsed);
  }

  if (!ctx.sync.is_realtime_factor_unlimited()) {
    auto width = ctx.sync.step_width().count();
    auto target = cloe::Duration(static_cast<uint64_t>(width / ctx.sync.realtime_factor()));
    padding = target - elapsed;
    if (padding.count() > 0) {
      std::this_thread::sleep_for(padding);
    } else {
      logger()->trace("Failing target realtime factor: {:.2f} < {:.2f}",
                      ctx.sync.achievable_realtime_factor(), ctx.sync.realtime_factor());
    }
  }

  auto guard = ctx.server->lock();
  ctx.statistics.cycle_time_ms.push_back(
      std::chrono::duration_cast<cloe::Milliseconds>(elapsed).count());
  ctx.statistics.padding_time_ms.push_back(
      std::chrono::duration_cast<cloe::Milliseconds>(padding).count());
  ctx.sync.increment_step();

  // Process all inserted triggers now.
  ctx.coordinator->process(ctx.sync);

  // We can pause the simulation between STEP_END and STEP_BEGIN.
  if (ctx.pause_execution) {
    return PAUSE;
  }

  return STEP_BEGIN;
}

// PAUSE --------------------------------------------------------------------------------------- //

StateId SimulationMachine::Pause::impl(SimulationContext& ctx) {
  if (state_machine()->previous_state() != PAUSE) {
    logger()->info("Pausing simulation...");
    logger()->info(R"(Send {"event": "pause", "action": "resume"} trigger to resume.)");
    logger()->debug(
        R"(For example: echo '{{"event": "pause", "action": "resume"}}' | curl -d @- http://localhost:{}/api/triggers/input)",
        ctx.config.server.listen_port);
    // If the server is not enabled, then the user probably won't be able to resume.
    if (!ctx.config.server.listen) {
      logger()->warn("Start temporary server.");
      ctx.server->start();
    }
  }

  {
    // Process all inserted triggers here, because the main loop is not running
    // while we are paused. Ideally, we should only allow triggers that are
    // destined for the pause state, although it might be handy to pause, allow
    // us to insert triggers, and then resume. Triggers that are inserted via
    // the web UI are just as likely to be incorrectly inserted as correctly.
    auto guard = ctx.server->lock();
    ctx.coordinator->process(ctx.sync);
  }

  // TODO(ben): Process triggers that come in so we can also conclude.
  // What kind of triggers do we want to allow? Should we also be processing
  // NEXT trigger events? How after pausing do we resume?
  ctx.callback_loop->trigger(ctx.sync);
  ctx.callback_pause->trigger(ctx.sync);
  std::this_thread::sleep_for(ctx.config.engine.polling_interval);

  if (ctx.pause_execution) {
    return PAUSE;
  }

  return RESUME;
}

// RESUME -------------------------------------------------------------------------------------- //

StateId SimulationMachine::Resume::impl(SimulationContext& ctx) {
  // TODO(ben): Eliminate the RESUME state and move this functionality into
  // the PAUSE state. This more closely matches the way we think about PAUSE
  // as a state vs. RESUME as a transition.
  logger()->info("Resuming simulation...");
  if (!ctx.config.server.listen) {
    logger()->warn("Stop temporary server.");
    ctx.server->stop();
  }
  ctx.callback_resume->trigger(ctx.sync);
  return STEP_BEGIN;
}

// STOP ---------------------------------------------------------------------------------------- //

StateId SimulationMachine::Stop::impl(SimulationContext& ctx) {
  logger()->info("Stopping simulation...");

  // If no other outcome has already been defined, then mark as "stopped".
  if (!ctx.outcome) {
    ctx.outcome = SimulationOutcome::Stopped;
  }

  ctx.callback_stop->trigger(ctx.sync);
  ctx.foreach_model([this, &ctx](cloe::Model& m, const char* type) {
    try {
      if (m.is_operational()) {
        logger()->debug("Stop {} {}", type, m.name());
        m.stop(ctx.sync);
      }
    } catch (std::exception& e) {
      logger()->error("Stopping {} {} failed: {}", type, m.name(), e.what());
    }
    return true;
  });
  ctx.progress.message = "execution complete";
  ctx.progress.execution.end();

  if (ctx.config.engine.keep_alive) {
    return KEEP_ALIVE;
  }
  return DISCONNECT;
}

// DISCONNECT ---------------------------------------------------------------------------------- //

StateId SimulationMachine::Disconnect::impl(SimulationContext& ctx) {
  logger()->debug("Disconnecting simulation...");
  ctx.foreach_model([](cloe::Model& m, const char*) {
    m.disconnect();
    return true;
  });
  logger()->info("Simulation disconnected.");
  return nullptr;
}

// SUCCESS ------------------------------------------------------------------------------------- //

StateId SimulationMachine::Success::impl(SimulationContext& ctx) {
  logger()->info("Simulation successful.");
  ctx.outcome = SimulationOutcome::Success;
  ctx.callback_success->trigger(ctx.sync);
  return STOP;
}

// FAIL ---------------------------------------------------------------------------------------- //

StateId SimulationMachine::Fail::impl(SimulationContext& ctx) {
  logger()->info("Simulation failed.");
  ctx.outcome = SimulationOutcome::Failure;
  ctx.callback_failure->trigger(ctx.sync);
  return STOP;
}

// RESET --------------------------------------------------------------------------------------- //

StateId SimulationMachine::Reset::impl(SimulationContext& ctx) {
  logger()->info("Resetting simulation...");
  ctx.callback_reset->trigger(ctx.sync);
  auto ok = ctx.foreach_model([this, &ctx](cloe::Model& m, const char* type) {
    try {
      logger()->debug("Reset {} {}", type, m.name());
      m.stop(ctx.sync);
      m.reset();
    } catch (std::exception& e) {
      logger()->error("Resetting {} {} failed: {}", type, m.name(), e.what());
      return false;
    }
    return true;
  });
  if (ok) {
    return CONNECT;
  } else {
    return ABORT;
  }
}

// KEEP_ALIVE ---------------------------------------------------------------------------------- //

StateId SimulationMachine::KeepAlive::impl(SimulationContext& ctx) {
  if (state_machine()->previous_state() != KEEP_ALIVE) {
    logger()->info("Keeping simulation alive...");
    logger()->info("Press [Ctrl+C] to disconnect.");
  }
  ctx.callback_pause->trigger(ctx.sync);
  std::this_thread::sleep_for(ctx.config.engine.polling_interval);
  return KEEP_ALIVE;
}

// ABORT --------------------------------------------------------------------------------------- //

StateId SimulationMachine::Abort::impl(SimulationContext& ctx) {
  const auto *previous_state = state_machine()->previous_state();
  if (previous_state == KEEP_ALIVE) {
    return DISCONNECT;
  } else if (previous_state == CONNECT) {
    ctx.outcome = SimulationOutcome::NoStart;
    return DISCONNECT;
  }

  logger()->info("Aborting simulation...");
  ctx.outcome = SimulationOutcome::Aborted;
  ctx.foreach_model([this](cloe::Model& m, const char* type) {
    try {
      logger()->debug("Abort {} {}", type, m.name());
      m.abort();
    } catch (std::exception& e) {
      logger()->error("Aborting {} {} failed: {}", type, m.name(), e.what());
    }
    return true;
  });
  return DISCONNECT;
}

// --------------------------------------------------------------------------------------------- //

Simulation::Simulation(cloe::Stack&& config, sol::state&& lua, const std::string& uuid)
    : config_(std::move(config))
    , lua_(std::move(lua))
    , logger_(cloe::logger::get("cloe"))
    , uuid_(uuid) {}

SimulationResult Simulation::run() {
  // Input:
  SimulationContext ctx{lua_.lua_state()};
  ctx.db = std::make_unique<cloe::DataBroker>(ctx.lua);
  ctx.server = make_server(config_.server);
  ctx.coordinator = std::make_unique<Coordinator>(ctx.lua, ctx.db.get());
  ctx.registrar = std::make_unique<Registrar>(ctx.server->server_registrar(), ctx.coordinator.get(),
                                              ctx.db.get());
  ctx.commander = std::make_unique<CommandExecuter>(logger());
  ctx.sync = SimulationSync(config_.simulation.model_step_width);
  ctx.config = config_;
  ctx.uuid = uuid_;
  ctx.report_progress = report_progress_;

  // Output:
  SimulationResult r;
  r.uuid = uuid_;
  r.config = ctx.config;
  r.set_output_dir();
  r.outcome = SimulationOutcome::NoStart;

  // Abort handler:
  SimulationMachine machine;
  abort_fn_ = [this, &ctx, &machine]() {
    static size_t requests = 0;

    logger()->info("Signal caught.");
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

  // Execution:
  try {
    // Start the server if enabled
    if (config_.server.listen) {
      ctx.server->start();
    }
    // Stream data to the requested file
    if (r.config.engine.output_file_data_stream) {
      auto filepath = r.get_output_filepath(*r.config.engine.output_file_data_stream);
      if (is_writable(filepath)) {
        ctx.server->init_stream(filepath.native());
      }
    }

    // Run pre-connect hooks
    ctx.commander->set_enabled(config_.engine.security_enable_hooks);
    ctx.commander->run_all(config_.engine.hooks_pre_connect);
    ctx.commander->set_enabled(config_.engine.security_enable_commands);

    // Run the simulation
    machine.run(ctx);
  } catch (cloe::ConcludedError& e) {
    // Nothing
  } catch (std::exception& e) {
    throw;
  }

  try {
    // Run post-disconnect hooks
    ctx.commander->set_enabled(config_.engine.security_enable_hooks);
    ctx.commander->run_all(config_.engine.hooks_post_disconnect);
  } catch (cloe::ConcludedError& e) {
    // TODO(ben): ensure outcome is correctly saved
  }

  // Wait for any running children to terminate.
  // (We could provide an option to time-out; this would involve using wait_for
  // instead of wait.)
  ctx.commander->wait_all();

  // TODO(ben): Preserve NoStart outcome.
  if (ctx.outcome) {
    r.outcome = *ctx.outcome;
  } else {
    r.outcome = SimulationOutcome::Aborted;
  }
  r.sync = ctx.sync;
  r.statistics = ctx.statistics;
  r.elapsed = ctx.progress.elapsed();
  r.triggers = ctx.coordinator->history();
  r.report = sol::object(ctx.lua["cloe"]["state"]["report"]);

  abort_fn_ = nullptr;
  return r;
}

size_t Simulation::write_output(const SimulationResult& r) const {
  if (r.output_dir) {
    logger()->debug("Using output path: {}", r.output_dir->native());
  }

  size_t files_written = 0;
  auto write_file = [&](auto filename, cloe::Json output) {
    if (!filename) {
      return;
    }

    boost::filesystem::path filepath = r.get_output_filepath(*filename);
    if (write_output_file(filepath, output)) {
      files_written++;
    }
  };

  write_file(r.config.engine.output_file_result, r);
  write_file(r.config.engine.output_file_config, r.config);
  write_file(r.config.engine.output_file_triggers, r.triggers);
  logger()->info("Wrote {} output files.", files_written);

  return files_written;
}

bool Simulation::write_output_file(const boost::filesystem::path& filepath,
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
  ofs << j.dump(2) << std::endl;
  return true;
}

bool Simulation::is_writable(const boost::filesystem::path& filepath) const {
  // Make sure we're not clobbering anything if we shouldn't.
  auto native = filepath.native();
  if (boost::filesystem::exists(filepath)) {
    if (!config_.engine.output_clobber_files) {
      logger()->error("Will not clobber file: {}", native);
      return false;
    }
    if (!boost::filesystem::is_regular_file(filepath)) {
      logger()->error("Cannot clobber non-regular file: {}", native);
      return false;
    }
  }

  // Make sure the directory exists.
  auto dirpath = filepath.parent_path();
  if (!boost::filesystem::is_directory(dirpath)) {
    bool ok = boost::filesystem::create_directories(dirpath);
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
