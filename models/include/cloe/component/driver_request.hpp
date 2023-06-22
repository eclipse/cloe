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
 * \file cloe/component/driver_request.hpp
 */

#pragma once

#include <fable/json.hpp> // for Json
#include <fable/utility/boost_optional.hpp>

#include <cloe/component.hpp>  // for Component

namespace cloe {

class DriverRequest : public Component {
 public:
  using Component::Component;
  DriverRequest() : Component("driver_request") {}
  virtual ~DriverRequest() noexcept = default;

  /**
   * Return driver-requested acceleration in [m/s^2].
   */
  virtual boost::optional<double> acceleration() const = 0;

  /**
   * Return true if acceleration request is available for this step.
   */
  virtual bool has_acceleration() const = 0;

  /**
   * Return driver-requested steering angle at front wheels in [rad].
   */
  virtual boost::optional<double> steering_angle() const = 0;

  /**
   * Return true if steering angle request is available for this step.
   */
  virtual bool has_steering_angle() const = 0;

  /**
   * Return sensor state as JSON.
   */
  fable::Json active_state() const override {
    return fable::Json{
        {"acceleration", acceleration()},
        {"steering_angle", steering_angle()},
    };
  }
};

/**
 * NopDriverRequest is an example no-op implementation of DriverRequest.
 */
class NopDriverRequest : public DriverRequest {
 public:
  using DriverRequest::DriverRequest;
  NopDriverRequest() : DriverRequest("nop_driver_request") {}
  virtual ~NopDriverRequest() noexcept {};

  boost::optional<double> acceleration() const override { return acceleration_; }
  bool has_acceleration() const override { return static_cast<bool>(acceleration_); }

  boost::optional<double> steering_angle() const override { return steering_angle_; }
  bool has_steering_angle() const override { return static_cast<bool>(steering_angle_); }

  Duration process(const Sync& sync) override {
    auto t = DriverRequest::process(sync);
    acceleration_.reset();
    steering_angle_.reset();
    return t;
  }

  void reset() override {
    DriverRequest::reset();
    acceleration_.reset();
    steering_angle_.reset();
  }

 protected:
  boost::optional<double> acceleration_{0.0};
  boost::optional<double> steering_angle_{0.0};
};

}  // namespace cloe
