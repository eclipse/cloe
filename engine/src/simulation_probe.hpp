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
 * \file simulation_probe.hpp
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include <fable/json.hpp>

#include "simulation_outcome.hpp"

namespace engine {

/**
 * SimulationProbe contains the results of a probe of the simulation
 * configuration.
 *
 * These fields are filled in from the PROBE state.
 *
 * This is primarily presented to the user as a single JSON output.
 */
struct SimulationProbe {
  SimulationOutcome outcome;

  /// Collection of errors from running the probe.
  std::vector<std::string> errors;

  /// UUID of the simulation, if any.
  std::string uuid;

  /// Map of plugin name -> plugin path.
  std::map<std::string, std::string> plugins;

  /// Map of vehicle name -> list of components.
  std::map<std::string, std::vector<std::string>> vehicles;

  /// List of trigger actions enrolled.
  std::map<std::string, fable::Json> trigger_actions;

  /// List of trigger events enrolled.
  std::map<std::string, fable::Json> trigger_events;

  /// List of HTTP endpoints that are available.
  std::vector<std::string> http_endpoints;

  /// Mapping from signal name to type.
  /// - @field name type help
  /// - @field name
  /// - @alias name
  std::map<std::string, std::string> signal_metadata;

  /// Complex JSON of test metadata, including (but not limited to):
  /// - test ID
  /// - user-supplied metadata
  fable::Json test_metadata;

  friend void to_json(fable::Json& j, const SimulationProbe& r) {
    j = fable::Json{
        {"uuid", r.uuid},
        {"plugins", r.plugins},
        {"vehicles", r.vehicles},
        {"trigger_actions", r.trigger_actions},
        {"trigger_events", r.trigger_events},
        {"http_endpoints", r.http_endpoints},
        {"signals", r.signal_metadata},
        {"tests", r.test_metadata},
    };
  }
};

}  // namespace engine
