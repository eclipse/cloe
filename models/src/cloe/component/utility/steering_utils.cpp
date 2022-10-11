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
 * \file cloe/component/utility/steering_utils.cpp
 * \see  cloe/component/utility/steering_utils.hpp
 */

#include <cassert>
#include <cmath>

#include <cloe/component/utility/steering_utils.hpp>

namespace cloe {
namespace utility {

double calculate_wheel_angle(const Geometry& geometry, WheelId wheel_id, double steering_angle) {
  // check if the input values are correct
  assert((geometry.wheel_base > 0) &&
         "it does not make sense to call calculate_wheel_angle with a wheel_base smaller than 0");
  assert((geometry.track_base > 0) &&
         "it does not make sense to call calculate_wheel_angle with a track_base smaller than 0");
  assert((wheel_id != WheelId::RearRight) &&
         "it does not make sense to call calculate_wheel_angle for the rear right wheel");
  assert((wheel_id != WheelId::RearLeft) &&
         "it does not make sense to call calculate_wheel_angle for the rear left wheel");

  // return 1.0 if wheel id is front left, otherwise return -1.0. This is due to the geometric relation.
  double sign = (wheel_id == WheelId::FrontLeft ? 1.0 : -1.0);

  // calculate tangent of the provided steering angle as an intermediate step.
  double tangent_steering_angle = std::tan(steering_angle);

  // This equation is the divisor part of the geometric relation for low speeds.
  double divisor =
      (1 - sign * 0.5 * tangent_steering_angle * geometry.track_base / geometry.wheel_base);

  return std::atan(tangent_steering_angle / divisor);
}

}  // namespace utility
}  // namespace cloe
