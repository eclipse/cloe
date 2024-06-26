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
 * \file simulation_state_success.cpp
 */

#include "simulation_context.hpp"  // for SimulationContext, SimulationOutcome
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

StateId SimulationMachine::Success::impl(SimulationContext& ctx) {
  logger()->info("Simulation successful.");
  ctx.outcome = SimulationOutcome::Success;
  ctx.callback_success->trigger(ctx.sync);
  return STOP;
}

}  // namespace engine
