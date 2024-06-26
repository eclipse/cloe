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
 * \file simulation_state_step_simulators.cpp
 */

#include <chrono>  // for duration_cast<>

#include <cloe/core/duration.hpp>  // for Duration
#include <cloe/model.hpp>          // for ModelReset, ...
#include <cloe/simulator.hpp>      // for Simulator
#include <cloe/vehicle.hpp>        // for Vehicle

#include "server.hpp"              // for ctx.server
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

StateId SimulationMachine::StepSimulators::impl(SimulationContext& ctx) {
  auto guard = ctx.server->lock();

  timer::DurationTimer<cloe::Duration> t([&ctx](cloe::Duration d) {
    auto ms = std::chrono::duration_cast<cloe::Milliseconds>(d);
    ctx.statistics.simulator_time_ms.push_back(ms.count());
  });

  // Call the simulator bindings:
  ctx.foreach_simulator([&ctx](cloe::Simulator& simulator) {
    try {
      cloe::Duration sim_time = simulator.process(ctx.sync);
      if (!simulator.is_operational()) {
        throw cloe::ModelStop("simulator {} no longer operational", simulator.name());
      }
      if (sim_time != ctx.sync.time()) {
        throw cloe::ModelError(
            "simulator {} did not progress to required time: got {}ms, expected {}ms",
            simulator.name(), sim_time.count() / 1'000'000, ctx.sync.time().count() / 1'000'000);
      }
    } catch (cloe::ModelReset& e) {
      throw;
    } catch (cloe::ModelStop& e) {
      throw;
    } catch (cloe::ModelAbort& e) {
      throw;
    } catch (cloe::ModelError& e) {
      throw;
    } catch (...) {
      throw;
    }
    return true;
  });

  // Clear vehicle cache
  ctx.foreach_vehicle([this, &ctx](cloe::Vehicle& v) {
    auto t = v.process(ctx.sync);
    if (t < ctx.sync.time()) {
      logger()->error("Vehicle ({}, {}) not progressing; simulation compromised!", v.id(),
                      v.name());
    }
    return true;
  });

  return STEP_CONTROLLERS;
}

}  // namespace engine
