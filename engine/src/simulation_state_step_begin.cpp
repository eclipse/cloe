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
 * \file simulation_state_step_begin.cpp
 */

#include <chrono>  // for duration_cast

#include "server.hpp"              // for Server::refresh_buffer
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

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

}  // namespace engine
