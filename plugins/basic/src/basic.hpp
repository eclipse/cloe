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
 * \file basic.hpp
 * \see  basic.cpp
 *
 * The basic controller provides an approximation of several ADAS functions.
 *
 *  - AEB (Automatic Emergency Braking)
 *  - ACC (Adaptive Cruise Control)
 *  - LKA (Lane Keeping Assist)
 */

#pragma once

#include <string>

#include <cloe/controller.hpp>  // for Controller, ControllerFactory, ...
#include <cloe/models.hpp>      // for CloeComponent

namespace cloe {
namespace controller {
namespace basic {

struct AccConfiguration : public Confable {
  std::string ego_sensor = to_string(CloeComponent::DEFAULT_EGO_SENSOR);
  std::string world_sensor = to_string(CloeComponent::DEFAULT_WORLD_SENSOR);
  std::string latlong_actuator = to_string(CloeComponent::DEFAULT_LATLONG_ACTUATOR);

  double limit_deceleration = 5.0;
  double limit_acceleration = 3.0;

  /*
  * Controller Parameters
  *
  * These parameters are heavily connected to the used vehicle model. For
  * example a VTD model has a very simple vehicle model and can follow
  * commands directly. Other vehicles models on the other hand may be pretty
  * slow in adapting to changes of the acceleration. Therefore the parameters
  * need to be adapted to have a more sufficient controller.
  *
  * Hint: For a better VTD control set the integrator parts from the variable
  * ki to 0.
  *
  * General Behaviour and Constraints:
  *
  * - ACC only works within one lane; the function cannot distinguish between
  *   lanes and does not take lane information into account.
  * - AEB can prevent car crash but sometimes interferes with the ACC,
  *   which can be especially degrading to ACC performance.
  * - ACC parameters are highly dependent on the vehicle dynamics and
  *   should be adapted accordingly. See the next two sections.
  *
  * VTD Behavior (with default control parameters):
  * - works fine for timegap safe
  * - works fine for timegap normal
  * - not tested with timegap fifty
  * - works fine for timegap crazy
  */
  double kd{5};
  double ki{0};
  double kp{0.8};

  /*
  * Another possibility to improve the control is to use different controller
  * parameters. One set for the speed control and one set for the distance
  * control
  */
  double kd_m = kd;
  double ki_m = ki;
  double kp_m = kp;

  CONFABLE_SCHEMA(AccConfiguration) {
    // clang-format off
    return Schema{
        {"ego_sensor",                        Schema(&ego_sensor, "ego sensor component to read from")},
        {"world_sensor",                      Schema(&world_sensor, "world_sensor component to read from")},
        {"latlong_actuator",                  Schema(&latlong_actuator, "actuator to write to")},
        {"limit_acceleration",                Schema(&limit_acceleration, "acceleration limit in [m/s^2]")},
        {"limit_deceleration",                Schema(&limit_deceleration, "how much deceleration is allowed, in [m/s^2]")},
        {"derivative_factor_speed_control",   Schema(&kd, "factor to tune the D term of the PID speed controller")},
        {"proportional_factor_speed_control", Schema(&kp, "factor to tune the P term of the PID speed controller")},
        {"integral_factor_speed_control",     Schema(&ki, "factor to tune the I term of the PID speed controller")},
        {"derivative_factor_dist_control",    Schema(&kd_m, "factor to tune the D term of the PID distance controller")},
        {"proportional_factor_dist_control",  Schema(&kp_m, "factor to tune the P term of the PID distance controller")},
        {"integral_factor_dist_control",      Schema(&ki_m, "factor to tune the I term of the PID distance controller")},
    };
    // clang-format on
  }
};

struct AebConfiguration : public Confable {
  bool enabled = true;
  std::string ego_sensor = to_string(CloeComponent::DEFAULT_EGO_SENSOR);
  std::string world_sensor = to_string(CloeComponent::DEFAULT_WORLD_SENSOR);
  std::string latlong_actuator = to_string(CloeComponent::DEFAULT_LATLONG_ACTUATOR);
  bool always_full_stop = false;
  double limit_deceleration = 8.0;

  CONFABLE_SCHEMA(AebConfiguration) {
    // clang-format off
    return Schema{
        {"enabled",            Schema(&enabled, "whether automatic emergency braking is enabled")},
        {"ego_sensor",         Schema(&ego_sensor, "ego sensor component to read from")},
        {"world_sensor",       Schema(&world_sensor, "world_sensor component to read from")},
        {"latlong_actuator",   Schema(&latlong_actuator, "actuator to write to")},
        {"always_full_stop",   Schema(&always_full_stop, "whether to brake to a full-stop on activation")},
        {"limit_deceleration", Schema(&limit_deceleration, "how much deceleration is allowed, in [m/s^2]")},
    };
    // clang-format on
  }
};

struct LkaConfiguration : public Confable {
  bool enabled = false;
  std::string world_sensor = to_string(CloeComponent::DEFAULT_WORLD_SENSOR);
  std::string latlong_actuator = to_string(CloeComponent::DEFAULT_LATLONG_ACTUATOR);
  double adjustment_rad = 0.02;
  double tolerance = 0.1;
  double lerp_factor = 0.1;

  CONFABLE_SCHEMA(LkaConfiguration) {
    // clang-format off
    return Schema{
        {"enabled",          Schema(&enabled, "whether lane keeping assist is enabled")},
        {"world_sensor",     Schema(&world_sensor, "world_sensor component to read from")},
        {"latlong_actuator", Schema(&latlong_actuator, "actuator to write to")},
        {"adjustment_rad",   Schema(&adjustment_rad, "wheel angle adjustment in [rad]")},
        {"tolerance",        Schema(&tolerance, "absolute tolerance in [m]")},
        {"lerp_factor",      Schema(&lerp_factor, "linear interpolation factor with domain (0-1]")},
    };
    // clang-format on
  }
};

struct BasicConfiguration : public Confable {
  AccConfiguration acc;
  AebConfiguration aeb;
  LkaConfiguration lka;
  std::string driver_request;

  void to_json(Json& j) const override {
    j = Json{
        {"acc", acc},
        {"aeb", aeb},
        {"lka", lka},
    };
  }

  CONFABLE_SCHEMA(BasicConfiguration) {
    // clang-format off
    return Schema{
        {"acc",            Schema(&acc, "ACC configuration")},
        {"aeb",            Schema(&aeb, "AEB configuration")},
        {"lka",            Schema(&lka, "LKA configuration")},
        {"driver_request", Schema(&driver_request, "component providing driver request")},
    };
    // clang-format on
  }
};

DEFINE_CONTROLLER_FACTORY(BasicFactory, BasicConfiguration, "basic",
                          "very basic vehicle controller")

}  // namespace basic
}  // namespace controller
}  // namespace cloe
