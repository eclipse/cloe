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

#include <csignal>  // for signal

#include "error_handler.hpp"     // for conclude_error
#include "main_commands.hpp"     // for ProbeOptions, handle_*
#include "simulation.hpp"        // for Simulation
#include "simulation_probe.hpp"  // for SimulationProbe

namespace engine {

// From main_commands.cpp:
std::string handle_uuid(const ProbeOptions& opt);
std::tuple<cloe::Stack, sol::state> handle_config(const ProbeOptions& opt, const std::vector<std::string>& filepaths);

int probe(const ProbeOptions& opt, const std::vector<std::string>& filepaths) {
  try {
    auto uuid = handle_uuid(opt);
    auto [stack, lua] = handle_config(opt, filepaths);
    auto lua_view = sol::state_view(lua.lua_state());

    // Create simulation:
    Simulation sim(std::move(stack), lua_view, uuid);
    GLOBAL_SIMULATION_INSTANCE = &sim;
    std::ignore = std::signal(SIGINT, handle_signal);

    // Run simulation:
    auto result = cloe::conclude_error(*opt.stack_options.error, [&]() { return sim.probe(); });
    *opt.output << cloe::Json(result).dump(opt.json_indent) << "\n" << std::flush;
    return as_exit_code(result.outcome, false);
  } catch (cloe::ConcludedError& e) {
    return EXIT_FAILURE;
  }
}

}  // namespace engine
