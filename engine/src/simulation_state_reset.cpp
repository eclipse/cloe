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
 * \file simulation_state_reset.cpp
 */

#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

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

}  // namespace engine
