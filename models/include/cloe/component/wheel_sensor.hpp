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
 * \file cloe/component/wheel_sensor.hpp
 */

#pragma once

#include <cloe/component.hpp>        // for Component
#include <cloe/component/wheel.hpp>  // for Wheel
#include <fable/json.hpp>            // for Json

namespace cloe {

class WheelSensor : public Component {
 public:
  using Component::Component;
  WheelSensor() : Component("wheel_sensor") {}
  virtual ~WheelSensor() noexcept = default;

  /**
   * Return front left wheel.
   */
  virtual Wheel wheel_fl() const = 0;

  /**
   * Return front right wheel.
   */
  virtual Wheel wheel_fr() const = 0;

  /**
   * Return rear left wheel.
   */
  virtual Wheel wheel_rl() const = 0;

  /**
   * Return rear right wheel.
   */
  virtual Wheel wheel_rr() const = 0;

  /**
   * Return sensor state as JSON.
   */
  fable::Json active_state() const override {
    return fable::Json{
        {"wheel_fl", wheel_fl()},
        {"wheel_fr", wheel_fr()},
        {"wheel_rl", wheel_rl()},
        {"wheel_rr", wheel_rr()},
    };
  }
};

/**
 * NopWheelSensor is an example no-op implementation of WheelSensor.
 */
class NopWheelSensor : public WheelSensor {
 public:
  using WheelSensor::WheelSensor;
  NopWheelSensor() : WheelSensor("nop_wheel_sensor") {}
  virtual ~NopWheelSensor() noexcept {};

  Wheel wheel_fl() const override { return wheel_fl_; };
  Wheel wheel_fr() const override { return wheel_fr_; };
  Wheel wheel_rl() const override { return wheel_rl_; };
  Wheel wheel_rr() const override { return wheel_rr_; };

  void reset() override {
    wheel_fl_ = Wheel();
    wheel_fr_ = Wheel();
    wheel_rl_ = Wheel();
    wheel_rr_ = Wheel();
  }

 protected:
  Wheel wheel_fl_{};
  Wheel wheel_fr_{};
  Wheel wheel_rl_{};
  Wheel wheel_rr_{};
};

}  // namespace cloe
