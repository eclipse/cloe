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
 * \file cloe/component/utility/ego_sensor_canon.hpp
 *
 * This file serves as a way of documenting how the objects are to be used
 * that are passed around components in Cloe.
 */

#pragma once
#ifndef CLOE_COMPONENT_UTILITY_EGO_SENSOR_CANON_HPP_
#define CLOE_COMPONENT_UTILITY_EGO_SENSOR_CANON_HPP_

#include <math.h>  // for fabs
#include <memory>  // for shared_ptr<>

#include <cloe/component/ego_sensor.hpp>  // for EgoSensor, Object

namespace cloe {
namespace utility {

/**
 * Provides methods that return canonical values from an EgoSensor.
 *
 * This can also be used to test whether an EgoSensor implementation is
 * correct. If you want to make use of these functions, you can wrap any
 * EgoSensor with EgoSensorCanon:
 *
 *    auto ego = EgoSensorCanon(veh->get<EgoSensor>(...));
 *
 * However, at some point it may be beneficial to optimize it away.
 */
class EgoSensorCanon : public EgoSensor {
 public:
  explicit EgoSensorCanon(std::shared_ptr<const EgoSensor> ego) : ego_(ego) {}
  const Object& sensed_state() const override { return ego_->sensed_state(); }
  double wheel_steering_angle() const override { return ego_->wheel_steering_angle(); }
  double driver_request_acceleration() const override {
    return ego_->driver_request_acceleration();
  }
  double driver_request_wheel_steering_angle() const override {
    return ego_->driver_request_wheel_steering_angle();
  }
  double steering_wheel_speed() const override { return ego_->steering_wheel_speed(); }
  double vehicle_length() const { return sensed_state().dimensions.x(); }
  double vehicle_width() const { return sensed_state().dimensions.y(); }
  double vehicle_height() const { return sensed_state().dimensions.z(); }

  /**
   * Return the ego velocity in meters per second [m/s].
   *
   *        ^
   *        | velocity in forward direction
   *
   *      +--+
   *      |  |
   *      |  |
   *      +--+
   *
   */
  double velocity_as_mps() const { return sensed_state().velocity.norm(); }

  /**
   * Return the ego velocity in kilometers per hour [km/h].
   */
  double velocity_as_kmph() const { return velocity_as_mps() * (60 * 60 / 1000.0); }

  /**
   * Return the ego acceleration in meters per second squared [m/s^2].
   */
  double acceleration_as_mpss() const { return sensed_state().acceleration.norm(); }

 protected:
  std::shared_ptr<const EgoSensor> ego_;
};

/**
 * Return the distance the object is in front of the ego.
 */
inline double distance_forward(const Object& o) { return o.pose.translation().x(); }

/**
 * Return the distance the object is to the right of the ego.
 */
inline double distance_starboard(const Object& o) { return -o.pose.translation().y(); }

/**
 * Return whether the object is in front of the ego.
 *
 * TODO(ben): There has got to be a better way to do this.
 */
inline bool is_object_fore(const Object& o) { return distance_forward(o) > 1.0E-9; }

/**
 * Return whether the object is behind the ego.
 *
 * TODO(ben): There has got to be a better way to do this.
 */
inline bool is_object_aft(const Object& o) { return distance_forward(o) < 1.0E-9; }

/**
 * Return whether the object is in the same "lane".
 *
 * TODO(ben): There has got to be a better way to do this. The current logic
 * runs a little like this:
 *
 *   If the center of the object is more than 2.5m away from our current
 *   position, then the vehicle is not in our lane. (Most personal automobiles
 *   are around 1.85m wide.)
 *
 * This is unfortunately not correct for a wide variety of situations:
 *
 *   - What if the road is curved?
 *   - What if we are changing the lane?
 *   - What if the lane is wider?
 *   - What if the lane is narrower?
 */
inline bool is_same_lane(const Object& o) { return fabs(distance_starboard(o)) < 2.5; }

/**
 * Returns the closest object that is in front of the ego.
 */
std::shared_ptr<Object> closest_forward(const Objects objects);

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_COMPONENT_UTILITY_EGO_SENSOR_CANON_HPP_
