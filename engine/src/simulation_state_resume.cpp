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
 * \file simulation_state_resume.cpp
 */

#include "server.hpp"              // for Server::stop, ...
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

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

}  // namespace engine
