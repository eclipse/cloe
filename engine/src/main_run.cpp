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

#include <csignal>  // for signal
#include <tuple>     // for tuple

#include "error_handler.hpp"      // for conclude_error
#include "main_commands.hpp"      // for RunOptions, handle_*
#include "simulation.hpp"         // for Simulation
#include "simulation_result.hpp"  // for SimulationResult
#include "stack.hpp"              // for Stack

namespace engine {

std::string handle_uuid(const RunOptions& opt);
std::tuple<cloe::Stack, sol::state> handle_config(const RunOptions& opt,
                                                  const std::vector<std::string>& filepaths);

int run(const RunOptions& opt, const std::vector<std::string>& filepaths) {
  try {
    auto uuid = handle_uuid(opt);
    auto [stack, lua] = handle_config(opt, filepaths);
    auto lua_view = sol::state_view(lua.lua_state());

    if (!opt.allow_empty) {
      stack.check_completeness();
    }
    if (!opt.output_path.empty()) {
      stack.engine.output_path = opt.output_path;
    }

    // Create simulation:
    Simulation sim(std::move(stack), lua_view, uuid);
    GLOBAL_SIMULATION_INSTANCE = &sim;
    std::ignore = std::signal(SIGINT, handle_signal);

    // Set options:
    sim.set_report_progress(opt.report_progress);

    // Run simulation:
    auto result = cloe::conclude_error(*opt.stack_options.error, [&]() { return sim.run(); });
    if (result.outcome == SimulationOutcome::NoStart) {
      // If we didn't get past the initialization phase, don't output any
      // statistics or write any files, just go home.
      return EXIT_FAILURE;
    }

    // Write results:
    if (opt.write_output) {
      sim.write_output(result);
    }
    *opt.output << cloe::Json(result).dump(opt.json_indent) << "\n" << std::flush;
    return as_exit_code(result.outcome, opt.require_success);
  } catch (cloe::ConcludedError& e) {
    return EXIT_FAILURE;
  }
}

}  // namespace engine
