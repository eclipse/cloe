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
 * \file simulation_result.hpp
 */

#pragma once

#include <string>

#include <cloe/core/duration.hpp>
#include <fable/json.hpp>

#include "simulation_outcome.hpp"
#include "simulation_statistics.hpp"
#include "simulation_sync.hpp"

namespace engine {

struct SimulationResult {
  SimulationOutcome outcome;

  /// Collection of errors from running the simulation.
  std::vector<std::string> errors;

  /// UUID of the simulation run.
  std::string uuid;

  /// Contains data regarding the time synchronization.
  SimulationSync sync;

  /// Contains the wall-clock time passed.
  cloe::Duration elapsed;

  /// Statistics regarding the simulation performance.
  SimulationStatistics statistics;

  /// The list of triggers run (i.e., the history).
  fable::Json triggers;

  /// The final report, as constructed from Lua.
  fable::Json report;

  friend void to_json(fable::Json& j, const SimulationResult& r) {
    j = fable::Json{
        {"elapsed", r.elapsed}, {"errors", r.errors},   {"outcome", r.outcome},
        {"report", r.report},   {"simulation", r.sync}, {"statistics", r.statistics},
        {"uuid", r.uuid},
    };
  }
};

}  // namespace engine
