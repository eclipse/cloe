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
 * \file cloe/component/ego_sensor.hpp
 */

#pragma once
#ifndef CLOE_COMPONENT_EGO_SENSOR_HPP_
#define CLOE_COMPONENT_EGO_SENSOR_HPP_

#include <cloe/component.hpp>         // for Component, Json
#include <cloe/component/object.hpp>  // for Object

namespace cloe {

class EgoSensor : public Component {
 public:
  using Component::Component;
  EgoSensor() : Component("ego_sensor") {}
  virtual ~EgoSensor() noexcept = default;

  /**
   * Return the sensed ego state.
   *
   * - The object's properties, such as velocity and acceleration,
   *   are in absolute coordinates.
   * - The returned pointer is invalid after clear_cache is called.
   * - The EgoSensor that returns the pointer manages the memory.
   */
  virtual const Object& sensed_state() const = 0;

  /**
   * Return the front left wheel steering angle in radians.
   */
  virtual double wheel_steering_angle() const = 0;

  /**
   * Return the requested longitudinal acceleration in m/s2.
   */
  virtual double driver_request_acceleration() const = 0;

  /**
   * Return the requested front left wheel steering angle in radians.
   */
  virtual double driver_request_wheel_steering_angle() const = 0;

  /**
   * Return the speed of the steering wheel rotation in radians/s.
   *
   * Positive values indicate clockwise rotation from the perspective of the
   * driver.
   *
   * FIXME(tobias): the way this is implemented in the vtd plugin is not
   * consistent with what the function name/documentation suggests. Instead of
   * the steering wheel speed, the driver-requested steering speed at the front
   * wheels is returned. Either change the interface or the implementation.
   */
  virtual double steering_wheel_speed() const = 0;

  /**
   * Return sensor state as JSON.
   */
  Json active_state() const override {
    return Json{
        {"sensed_state", this->sensed_state()},
        {"wheel_steering_angle", this->wheel_steering_angle()},
    };
  }
};

/**
 * NopEgoSensor is an example no-op implementation of EgoSensor.
 */
class NopEgoSensor : public EgoSensor {
 public:
  using EgoSensor::EgoSensor;
  NopEgoSensor() : EgoSensor("nop_ego_sensor") {}
  virtual ~NopEgoSensor() noexcept = default;
  const Object& sensed_state() const override { return obj_; }
  double wheel_steering_angle() const override { return angle_; }
  double driver_request_acceleration() const override { return driver_req_accel_; }
  double driver_request_wheel_steering_angle() const override { return driver_req_angle_; }
  virtual double steering_wheel_speed() const { return steering_wheel_speed_; }
  void reset() override {
    obj_ = Object{};
    angle_ = 0.0;
    driver_req_angle_ = 0.0;
    steering_wheel_speed_ = 0.0;
  }

 protected:
  Object obj_{};
  double angle_{0.0};
  double driver_req_accel_{0.0};
  double driver_req_angle_{0.0};
  double steering_wheel_speed_{0.0};
};

}  // namespace cloe

#endif  // CLOE_COMPONENT_EGO_SENSOR_HPP_
