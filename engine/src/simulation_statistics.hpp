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
 * \file simulation_statistics.hpp
 */

#pragma once

#include <cloe/utility/statistics.hpp>  // for Accumulator
#include <fable/json.hpp>

namespace engine {

struct SimulationStatistics {
  cloe::utility::Accumulator engine_time_ms;
  cloe::utility::Accumulator cycle_time_ms;
  cloe::utility::Accumulator simulator_time_ms;
  cloe::utility::Accumulator controller_time_ms;
  cloe::utility::Accumulator padding_time_ms;
  cloe::utility::Accumulator controller_retries;

  void reset() {
    engine_time_ms.reset();
    cycle_time_ms.reset();
    simulator_time_ms.reset();
    controller_time_ms.reset();
    padding_time_ms.reset();
    controller_retries.reset();
  }

  friend void to_json(fable::Json& j, const SimulationStatistics& s) {
    j = fable::Json{
        {"engine_time_ms", s.engine_time_ms},         {"simulator_time_ms", s.simulator_time_ms},
        {"controller_time_ms", s.controller_time_ms}, {"padding_time_ms", s.padding_time_ms},
        {"cycle_time_ms", s.cycle_time_ms},           {"controller_retries", s.controller_retries},
    };
  }
};

}  // namespace engine
