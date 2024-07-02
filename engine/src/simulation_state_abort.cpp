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
 * \file simulation_state_abort.cpp
 */

#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

StateId SimulationMachine::Abort::impl(SimulationContext& ctx) {
  const auto* previous_state = state_machine()->previous_state();
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

}  // namespace engine
