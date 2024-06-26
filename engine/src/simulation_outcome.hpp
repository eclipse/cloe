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
 * \file simulation_outcome.hpp
 */

#pragma once

#include <map>  // for map<>

#include <fable/enum.hpp>  // for ENUM_SERIALIZATION

namespace engine {

/**
 * SimulationOutcome describes the possible outcomes a simulation can have.
 */
enum class SimulationOutcome {
  NoStart,  ///< Simulation unable to start.
  Aborted,  ///< Simulation aborted due to technical problems or interrupt.
  Stopped,  ///< Simulation concluded, but without valuation.
  Failure,  ///< Simulation explicitly concluded with failure.
  Success,  ///< Simulation explicitly concluded with success.
  Probing,  ///< Simulation started briefly to gather specific information.
};

// If possible, the following exit codes should not be used as they are used
// by the Bash shell, among others: 1-2, 126-165, and 255. That leaves us
// primarily with the range 3-125, which should suffice for our purposes.
// The following exit codes should not be considered stable.
#define EXIT_OUTCOME_SUCCESS EXIT_SUCCESS  // normally 0
#define EXIT_OUTCOME_UNKNOWN EXIT_FAILURE  // normally 1
#define EXIT_OUTCOME_NOSTART 4             // 0b.....1..
#define EXIT_OUTCOME_STOPPED 8             // 0b....1...
#define EXIT_OUTCOME_FAILURE 9             // 0b....1..1
#define EXIT_OUTCOME_ABORTED 16            // 0b...1....

// clang-format off
ENUM_SERIALIZATION(SimulationOutcome, ({
    {SimulationOutcome::Aborted, "aborted"},
    {SimulationOutcome::NoStart, "no-start"},
    {SimulationOutcome::Failure, "failure"},
    {SimulationOutcome::Success, "success"},
    {SimulationOutcome::Stopped, "stopped"},
    {SimulationOutcome::Probing, "probing"},
}))
// clang-format on

inline int as_exit_code(SimulationOutcome outcome, bool require_success = true) {
  switch (outcome) {
    case SimulationOutcome::Success:
      return EXIT_OUTCOME_SUCCESS;
    case SimulationOutcome::Stopped:
      return (require_success ? EXIT_OUTCOME_STOPPED : EXIT_OUTCOME_SUCCESS);
    case SimulationOutcome::Aborted:
      return EXIT_OUTCOME_ABORTED;
    case SimulationOutcome::NoStart:
      return EXIT_OUTCOME_NOSTART;
    case SimulationOutcome::Failure:
      return EXIT_OUTCOME_FAILURE;
    case SimulationOutcome::Probing:
      return EXIT_OUTCOME_SUCCESS;
    default:
      return EXIT_OUTCOME_UNKNOWN;
  }
}

}  // namespace engine
