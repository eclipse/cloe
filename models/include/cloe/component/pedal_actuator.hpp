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
 * \file cloe/component/pedal_actuator.hpp
 */

#pragma once

#include <cloe/component/actuator.hpp>  // for Actuator
#include <fable/json.hpp>               // for Json

namespace cloe {

struct PedalRequest {
  /*
   * Requested status of the gas pedal with no unit.
   *
   * The range goes from 0 (unpressed) to 1 (fully pressed).
   */
  double gas_pedal_position{0.0};

  /*
   * Requested status of the brake pedal with no unit.
   *
   * The range goes from 0 (unpressed) to 1 (fully pressed).
   */
  double brake_pedal_position{0.0};

  friend void to_json(fable::Json& j, const PedalRequest& p) {
    j = fable::Json{
        {"gas_pedal_position", p.gas_pedal_position},
        {"brake_pedal_position", p.brake_pedal_position},
    };
  }
};

class PedalActuator : public Actuator<PedalRequest> {
 public:
  using Actuator::Actuator;
  PedalActuator() : Actuator("pedal_actuator") {}
  virtual ~PedalActuator() noexcept = default;
};

}  // namespace cloe
