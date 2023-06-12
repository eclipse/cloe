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
 * \file cloe/component/powertrain_sensor.hpp
 */

#pragma once

#include <cloe/component.hpp>  // for Component
#include <fable/json.hpp>      // for Json

namespace cloe {

class PowertrainSensor : public Component {
 public:
  using Component::Component;
  PowertrainSensor() : Component("powertrain_sensor") {}
  virtual ~PowertrainSensor() noexcept = default;

  /**
   * Return the position of the acceleration pedal with no unit.
   *
   * The range goes from 0 (unpressed) to 1 (fully pressed).
   */
  virtual double pedal_position_acceleration() const = 0;

  /**
   * Return the gear transmission.
   *
   * The sign of this field is linked to the mode of the gear
   * - positive: driving forward (e.g. a value of 3 means the third gear in driving forward mode)
   * - 0: means that the gear lever is in neutral position
   * - negative: reverse mode (e.g. a value of -1 means the first gear in reverse mode)
   * - int max: means that the transmission is in parking position (can be accessed via std::numeric_limits<int>::max())
   */
  virtual int gear_transmission() const = 0;

  /**
   * Return sensor state as JSON.
   */
  fable::Json active_state() const override {
    return fable::Json{
        {"pedal_position_acceleration", pedal_position_acceleration()},
        {"gear_transmission", gear_transmission()},
    };
  }
};

/**
 * NopPowertrainSensor is an example no-op implementation of PowertrainSensor.
 */
class NopPowertrainSensor : public PowertrainSensor {
 public:
  using PowertrainSensor::PowertrainSensor;
  NopPowertrainSensor() : PowertrainSensor("nop_powertrain_sensor") {}
  virtual ~NopPowertrainSensor() noexcept {};

  double pedal_position_acceleration() const override { return pedal_position_acceleration_; }

  int gear_transmission() const override { return gear_transmission_; }

  void reset() override {
    pedal_position_acceleration_ = 0.0;
    gear_transmission_ = 0;
  }

 protected:
  double pedal_position_acceleration_{0.0};
  int gear_transmission_{0};
};

}  // namespace cloe
