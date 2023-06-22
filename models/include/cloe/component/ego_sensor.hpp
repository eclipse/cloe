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

#include <cloe/component.hpp>         // for Component
#include <cloe/component/object.hpp>  // for Object

#include <fable/json.hpp> // for Json

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
   * Return the speed of the steering wheel rotation in radians/s.
   *
   * Positive values indicate clockwise rotation from the perspective of the
   * driver.
   */
  virtual double steering_wheel_speed() const = 0;

  /**
   * Return sensor state as JSON.
   */
  fable::Json active_state() const override {
    return fable::Json{
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
  double steering_wheel_speed() const override { return steering_wheel_speed_; }
  void reset() override {
    obj_ = Object{};
    angle_ = 0.0;
    steering_wheel_speed_ = 0.0;
  }

 protected:
  Object obj_{};
  double angle_{0.0};
  double steering_wheel_speed_{0.0};
};

}  // namespace cloe
