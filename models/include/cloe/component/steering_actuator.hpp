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
 * \file cloe/component/steering_actuator.hpp
 */

#pragma once

#include <cloe/component/actuator.hpp>  // for Actuator
#include <fable/json.hpp>               // for Json

namespace cloe {

struct SteeringRequest {
  /**
   * Front center wheel angle with regards to z-Axis in [radians].
   */
  double steering_angle_front{0.0};

  /**
   * Rear center wheel angle with regards to z-Axis in [radians].
   */
  double steering_angle_rear{0.0};

  friend void to_json(fable::Json& j, const SteeringRequest& s) {
    j = fable::Json{
        {"steering_angle_front", s.steering_angle_front},
        {"steering_angle_rear", s.steering_angle_rear},
    };
  }
};

class SteeringActuator : public Actuator<SteeringRequest> {
 public:
  using Actuator::Actuator;
  SteeringActuator() : Actuator("steering_actuator") {}
  virtual ~SteeringActuator() noexcept = default;
};

}  // namespace cloe
