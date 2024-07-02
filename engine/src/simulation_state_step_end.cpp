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
 * \file simulation_state_step_end.cpp
 */

#include <chrono>   // for duration_cast
#include <cstdint>  // uint64_t
#include <thread>   // sleep_for

#include <cloe/core/duration.hpp>  // for Duration

#include "coordinator.hpp"         // for Coordinator::process
#include "server.hpp"              // for Server::lock, ...
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

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

}  // namespace engine
