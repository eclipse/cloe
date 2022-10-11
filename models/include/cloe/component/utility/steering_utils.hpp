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
 * \file cloe/component/utility/steering_utils.hpp
 */

#pragma once
#ifndef CLOE_COMPONENT_UTILITY_STEERING_UTILS_HPP_
#define CLOE_COMPONENT_UTILITY_STEERING_UTILS_HPP_

namespace cloe {
namespace utility {

struct Geometry {
  /// Distance between the front and the rear axle in [m].
  double wheel_base{0.0};

  /// Distance between the left wheel and the right wheel in [m].
  double track_base{0.0};
};

enum class WheelId : unsigned int { FrontLeft = 0, FrontRight, RearLeft, RearRight };

/**
 * This function translates a steering angle from the center of the axle to the individual steering angle of a wheel.
 *
 * This function is based on the Ackermann steering geometry, see [https://en.wikipedia.org/wiki/Ackermann_steering_geometry, accessed 28.10.2022]
 * The calculation only works for the following assumptions, as it assumes that the centre of rotation is on the same level with the rear wheels:
 *     - low speed
 *     - no rear steering
 * Detailed explanations can be found in basic vehicle dynamics literature, e.g. chapter 4.2 of the Steering Handbook 
 * by Harrer and Pfeffer [https://link.springer.com/book/10.1007/978-3-319-05449-0]
 * 
 * The function needs a wheel_base, a track_base (both stored in Geometry), the wheel_id and the steering_angle in the center of the axle.
 * The wheel_id is the ID you want to calculate the angle for, i.e. either FrontLeft or FrontRight.
 * The function returns the value of the steering_angle of the requested wheel.
 */
double calculate_wheel_angle(const Geometry& geometry, WheelId wheel_id, double steering_angle);

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_COMPONENT_UTILITY_STEERING_UTILS_HPP_
