/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file cloe/component/gearbox_actuator.hpp
 */

#pragma once

#include <cloe/component/actuator.hpp>  // for Actuator
#include <fable/json.hpp>               // for Json

namespace cloe {

struct GearboxRequest {
  /*
   * Holds the requested gear selector position.
   *
   * The sign of this field is linked to the mode of the gear
   * - positive: driving forward (e.g. a value of 3 means to request the third gear in driving forward mode)
   * - 0: means that the gear lever is requested to be in neutral position
   * - negative: reverse mode (e.g. a value of -1 means the first gear in reverse mode is requested)
   * - int8_t max: means that the transmission is in parking position (can be accessed via std::numeric_limits<int8_t>::max())
   */
  int8_t gear_selector{0};

  friend void to_json(fable::Json& j, const GearboxRequest& g) {
    j = fable::Json{{"gear_selector", g.gear_selector}};
  }
};

class GearboxActuator : public Actuator<GearboxRequest> {
 public:
  using Actuator::Actuator;
  GearboxActuator() : Actuator("gearbox_actuator") {}
  virtual ~GearboxActuator() noexcept = default;
};

}  // namespace cloe
