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
 * \file simulation_state_step_controllers.cpp
 */

#include "server.hpp"              // for Server::lock, ...
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

StateId SimulationMachine::StepControllers::impl(SimulationContext& ctx) {
  auto guard = ctx.server->lock();

  timer::DurationTimer<cloe::Duration> t([&ctx](cloe::Duration d) {
    auto ms = std::chrono::duration_cast<cloe::Milliseconds>(d);
    ctx.statistics.controller_time_ms.push_back(ms.count());
  });

  // We can only erase from ctx.controllers when we have access to the
  // iterator itself, otherwise we get undefined behavior. So we save
  // the names of the controllers we want to erase from the list.
  std::vector<std::string> controllers_to_erase;

  // Call each controller and handle any errors that might occur.
  ctx.foreach_controller([this, &ctx, &controllers_to_erase](cloe::Controller& ctrl) {
    if (!ctrl.has_vehicle()) {
      // Skip this controller
      return true;
    }

    // Keep calling the ctrl until it has caught up the current time.
    cloe::Duration ctrl_time;
    try {
      int64_t retries = 0;
      for (;;) {
        ctrl_time = ctrl.process(ctx.sync);

        // If we are underneath our target, sleep and try again.
        if (ctrl_time < ctx.sync.time()) {
          this->logger()->warn("Controller {} not progressing, now at {}", ctrl.name(),
                               cloe::to_string(ctrl_time));

          // If a controller is misbehaving, we might get stuck in a loop.
          // If this happens more than some random high number, then throw
          // an error.
          if (retries == ctx.config.simulation.controller_retry_limit) {
            throw cloe::ModelError{"controller not progressing to target time {}",
                                   cloe::to_string(ctx.sync.time())};
          }

          // Otherwise, sleep and try again.
          std::this_thread::sleep_for(ctx.config.simulation.controller_retry_sleep);
          ++retries;
        } else {
          ctx.statistics.controller_retries.push_back(static_cast<double>(retries));
          break;
        }
      }
    } catch (cloe::ModelReset& e) {
      this->logger()->error("Controller {} reset: {}", ctrl.name(), e.what());
      this->state_machine()->reset();
      return false;
    } catch (cloe::ModelStop& e) {
      this->logger()->error("Controller {} stop: {}", ctrl.name(), e.what());
      this->state_machine()->stop();
      return false;
    } catch (cloe::ModelAbort& e) {
      this->logger()->error("Controller {} abort: {}", ctrl.name(), e.what());
      this->state_machine()->abort();
      return false;
    } catch (cloe::Error& e) {
      this->logger()->error("Controller {} died: {}", ctrl.name(), e.what());
      if (e.has_explanation()) {
        this->logger()->error("Note:\n{}", e.explanation());
      }
      if (ctx.config.simulation.abort_on_controller_failure) {
        this->logger()->error("Aborting thanks to controller {}", ctrl.name());
        this->state_machine()->abort();
        return false;
      } else {
        this->logger()->warn("Continuing without controller {}", ctrl.name());
        ctrl.abort();
        ctrl.disconnect();
        controllers_to_erase.push_back(ctrl.name());
        return true;
      }
    } catch (...) {
      this->logger()->critical("Controller {} encountered a fatal error.", ctrl.name());
      throw;
    }

    // Write a notice if the controller is ahead of the simulation time.
    cloe::Duration ctrl_ahead = ctrl_time - ctx.sync.time();
    if (ctrl_ahead.count() > 0) {
      this->logger()->warn("Controller {} is ahead by {}", ctrl.name(),
                           cloe::to_string(ctrl_ahead));
    }

    // Continue with next controller.
    return true;
  });

  // Remove any controllers that we want to continue without.
  for (const auto& ctrl : controllers_to_erase) {
    ctx.controllers.erase(ctrl);
  }

  return STEP_END;
}

}  // namespace engine
