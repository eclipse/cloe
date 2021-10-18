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
 * \file cloe/component/latlong_actuator.hpp
 *
 * Currently, only LatLongActuator is in use, however in the future, the
 * Actuator components may become more interesting.
 */

#pragma once
#ifndef CLOE_COMPONENT_LATLONG_ACTUATOR_HPP_
#define CLOE_COMPONENT_LATLONG_ACTUATOR_HPP_

#include <boost/optional.hpp>         // for optional<>
#include <fable/json/with_boost.hpp>  // for to_json

#include <cloe/component.hpp>                // for Component, Json
#include <cloe/utility/actuation_level.hpp>  // for ActuationLevel

namespace cloe {

class LatLongActuator : public Component {
 public:
  using Component::Component;
  LatLongActuator() : Component("lat_long_actuator") {}
  virtual ~LatLongActuator() noexcept = default;

  /**
   * Set the vehicle target acceleration in m/s^2.
   * The time that the vehicle requires to reach this acceleration is undefined.
   */
  virtual void set_acceleration(double a) {
    target_acceleration_ = a;
    level_.set_long();
  }

  boost::optional<double> acceleration() const { return target_acceleration_; }

  /**
   * Return true if set_acceleration was called for the current step.
   */
  bool is_acceleration() const { return static_cast<bool>(target_acceleration_); }

  /**
   * Set the target steering angle of the wheels in rad.
   * The time that the vehicle requires to reach this steering angle is undefined.
   * In most cases, the front left wheel defines the angle, and the time to target
   * steering angle is zero.
   */
  virtual void set_steering_angle(double a) {
    target_steering_angle_ = a;
    level_.set_lat();
  }

  boost::optional<double> steering_angle() const { return target_steering_angle_; }

  /**
   * Return true if set_target_steering_angle was called for the current step.
   */
  bool is_steering_angle() const { return static_cast<bool>(target_steering_angle_); }

  /**
   * Return a single enum summarizing the current actuation level of control.
   *
   * Useful for tracking changes.
   */
  utility::ActuationLevel actuation_level() const { return level_; }

  /**
   * Write the JSON representation of an Actuator into j.
   *
   * Currently, the API is unstable, because we don't have access to any real
   * data.
   */
  Json active_state() const override {
    return Json{
        {"target_acceleration", target_acceleration_},
        {"target_steering_angle", target_steering_angle_},
        {"actuation_label", level_.to_human_cstr()},
    };
  }

  Duration process(const Sync& sync) override {
    auto t = Component::process(sync);
    target_acceleration_.reset();
    target_steering_angle_.reset();
    level_.set_none();
    return t;
  }

  void reset() override {
    Component::reset();
    target_acceleration_.reset();
    target_steering_angle_.reset();
    level_.set_none();
  }

 protected:
  utility::ActuationLevel level_;
  boost::optional<double> target_acceleration_;
  boost::optional<double> target_steering_angle_;
};

template <typename T = double>
class Actuator : public Component {
 public:
  using Component::Component;
  virtual ~Actuator() noexcept = default;

  virtual void set(T a) { target_ = a; }
  virtual bool is_set() { return static_cast<bool>(target_); }
  virtual T get() { return *target_; }

  Json active_state() const override { return Json{"target", target_}; }

  Duration process(const Sync& sync) override {
    auto t = Component::process(sync);
    target_.reset();
    return t;
  }

  void reset() override {
    Component::reset();
    target_.reset();
  }

 protected:
  boost::optional<T> target_;
};

class LongActuator : public Actuator<double> {
 public:
  using Actuator::Actuator;
  LongActuator() : Actuator("long_actuator") {}
  virtual ~LongActuator() noexcept = default;
};

class LatActuator : public Actuator<double> {
 public:
  using Actuator::Actuator;
  LatActuator() : Actuator("lat_actuator") {}
  virtual ~LatActuator() noexcept = default;
};

}  // namespace cloe

#endif  // CLOE_COMPONENT_LATLONG_ACTUATOR_HPP_
