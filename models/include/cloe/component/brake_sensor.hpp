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
 * \file cloe/component/brake_sensor.hpp
 */

#pragma once

#include <cloe/component.hpp>  // for Component
#include <fable/json.hpp>      // for Json

namespace cloe {

class BrakeSensor : public Component {
 public:
  using Component::Component;
  BrakeSensor() : Component("brake_sensor") {}
  virtual ~BrakeSensor() noexcept = default;

  /**
   * Return the position of the brake pedal with no unit.
   *
   * The range goes from 0 (unpressed) to 1 (fully pressed).
   */
  virtual double pedal_position_brake() const = 0;

  /**
   * Return sensor state as JSON.
   */
  fable::Json active_state() const override {
    return fable::Json{{"pedal_position_brake", pedal_position_brake()}};
  }
};

/**
 * NopBrakeSensor is an example no-op implementation of BrakeSensor.
 */
class NopBrakeSensor : public BrakeSensor {
 public:
  using BrakeSensor::BrakeSensor;
  NopBrakeSensor() : BrakeSensor("nop_brake_sensor") {}
  virtual ~NopBrakeSensor() noexcept {};

  double pedal_position_brake() const override { return pedal_position_brake_; }

  void reset() override { pedal_position_brake_ = 0.0; }

 protected:
  double pedal_position_brake_{0.0};
};

}  // namespace cloe
