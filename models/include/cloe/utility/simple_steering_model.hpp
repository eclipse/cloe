/*
 * Copyright 2020 Robert Bosch GmbH
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
 * \file cloe/utility/simple_steering_model.hpp
 */

#pragma once
#ifndef CLOE_UTILITY_SIMPLE_STEERING_MODEL_HPP_
#define CLOE_UTILITY_SIMPLE_STEERING_MODEL_HPP_

namespace cloe {
namespace utility {

/**
 * This steering model models the front-wheel angle based on the
 * torque applied at the steering wheel and the current vehicle
 * veloocity.
 *
 * This model must be updated before values are read from it.
 */
class SimpleSteeringModel {
 public:
  SimpleSteeringModel() = default;
  explicit SimpleSteeringModel(double delta_time_s) : dt_(delta_time_s) {}
  ~SimpleSteeringModel() = default;

  /**
   * Update the front-wheel steering angle based on the steering-wheel torque
   * and the vehicle velocity.
   *
   * Note that this updates the model over time, so that each call assumes that
   * a certain amount of time is.
   *
   * \param steering_torque in [Nm]
   * \param long_velocity in [m/s]
   */
  void update_model(double steering_torque, double long_velocity) {
    // Gain depending on vehicle velocity, to  reduce oscillations
    //  ^ v_gain
    // 1|*
    //  |  *
    //  |    *
    //  |      *
    //  |- - - - * * * * * MIN
    //  |--------|-------> velocity
    //  0       30 m/s
    double v_gain = -0.03 * long_velocity + 1;

    // Minimize to value
    if (v_gain <= 0.1) {
      v_gain = 0.1;
    }

    // target_steering_angle = target_steering_angle (t-1) + K * K_Ego_velocity_vx * dt * torque
    steering_angle_ = steering_angle_ + 0.1 * v_gain * dt_ * steering_torque;
  }

  /**
   * Return the steering angle at the front-wheel in [rad].
   */
  double steering_angle() const { return steering_angle_; }

 private:
  double dt_{0.02};
  double steering_angle_{0.0};
};

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_SIMPLE_STEERING_MODEL_HPP_
