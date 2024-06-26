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
 * \file simulation_state_disconnect.cpp
 */

#include <fable/utility/sol.hpp>  // for to_json
#include <sol/object.hpp>         // for object

#include "coordinator.hpp"         // for Coordinator::history
#include "lua_api.hpp"             // for luat_cloe_engine_state
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine
#include "simulation_result.hpp"   // for SimulationResult

namespace engine {

StateId SimulationMachine::Disconnect::impl(SimulationContext& ctx) {
  logger()->debug("Disconnecting simulation...");
  ctx.foreach_model([](cloe::Model& m, const char*) {
    m.disconnect();
    return true;
  });
  logger()->info("Simulation disconnected.");

  // Gather up the simulation results.
  auto result = SimulationResult();
  result.outcome = ctx.outcome.value_or(SimulationOutcome::Aborted);
  result.uuid = ctx.uuid;
  result.sync = ctx.sync;
  result.statistics = ctx.statistics;
  result.elapsed = ctx.progress.elapsed();
  result.triggers = ctx.coordinator->history();
  result.report = sol::object(cloe::luat_cloe_engine_state(ctx.lua)["report"]);
  ctx.result = result;

  return nullptr;
}

}  // namespace engine
