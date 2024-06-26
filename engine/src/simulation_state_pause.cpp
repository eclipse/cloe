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
 * \file simulation_state_pause.cpp
 */

#include <thread>  // for this_thread

#include "coordinator.hpp"         // for Coordinator::process
#include "server.hpp"              // for Server::...
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

StateId SimulationMachine::Pause::impl(SimulationContext& ctx) {
  if (state_machine()->previous_state() != PAUSE) {
    logger()->info("Pausing simulation...");
    logger()->info(R"(Send {"event": "pause", "action": "resume"} trigger to resume.)");
    logger()->debug(
        R"(For example: echo '{{"event": "pause", "action": "resume"}}' | curl -d @- http://localhost:{}/api/triggers/input)",
        ctx.config.server.listen_port);

    // If the server is not enabled, then the user probably won't be able to resume.
    if (!ctx.config.server.listen) {
      logger()->warn("Start temporary server.");
      ctx.server->start();
    }
  }

  {
    // Process all inserted triggers here, because the main loop is not running
    // while we are paused. Ideally, we should only allow triggers that are
    // destined for the pause state, although it might be handy to pause, allow
    // us to insert triggers, and then resume. Triggers that are inserted via
    // the web UI are just as likely to be incorrectly inserted as correctly.
    auto guard = ctx.server->lock();
    ctx.coordinator->process(ctx.sync);
  }

  // TODO(ben): Process triggers that come in so we can also conclude.
  // What kind of triggers do we want to allow? Should we also be processing
  // NEXT trigger events? How after pausing do we resume?
  ctx.callback_loop->trigger(ctx.sync);
  ctx.callback_pause->trigger(ctx.sync);
  std::this_thread::sleep_for(ctx.config.engine.polling_interval);

  if (ctx.pause_execution) {
    return PAUSE;
  }

  return RESUME;
}

}  // namespace engine
