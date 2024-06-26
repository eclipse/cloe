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
 * \file simulation_state_stop.cpp
 */

#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

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

}  // namespace engine
