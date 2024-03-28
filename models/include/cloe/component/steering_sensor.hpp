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
 * \file cloe/component/steering_sensor.hpp
 */

#pragma once

#include <cloe/component.hpp>  // for Component
#include <fable/json.hpp>      // for Json

namespace cloe {

class SteeringSensor : public Component {
 public:
  using Component::Component;
  SteeringSensor() : Component("steering_sensor") {}
  virtual ~SteeringSensor() noexcept = default;

  /**
   * Return curvature of ego vehicle track in [1/m].
   */
  virtual double curvature() const = 0;

  /**
   * Return sensor state as JSON.
   */
  fable::Json active_state() const override {
    return fable::Json{
        {"curvature", curvature()},
    };
  }
};

/**
 * NopSteeringSensor is an example no-op implementation of SteeringSensor.
 */
class NopSteeringSensor : public SteeringSensor {
 public:
  using SteeringSensor::SteeringSensor;
  NopSteeringSensor() : SteeringSensor("nop_steering_sensor") {}
  virtual ~NopSteeringSensor() noexcept {};

  double curvature() const override { return curvature_; }
  void reset() override { curvature_ = 0.0; }

 protected:
  double curvature_{0.0};
};

}  // namespace cloe
