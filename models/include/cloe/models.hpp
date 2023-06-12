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
 * \file cloe/models.hpp
 */

#pragma once

#include <string>  // for string

#include <fable/enum.hpp>  // for ENUM_SERIALIZATION

namespace cloe {

enum class CloeComponent {
  // Groundtruth sensors should never be modified/replaced.
  GROUNDTRUTH_EGO_SENSOR,
  GROUNDTRUTH_POWERTRAIN_SENSOR,
  GROUNDTRUTH_BRAKE_SENSOR,
  GROUNDTRUTH_WHEEL_SENSOR,
  GROUNDTRUTH_STEERING_SENSOR,
  GROUNDTRUTH_WORLD_SENSOR,
  GROUNDTRUTH_LANE_SENSOR,
  GROUNDTRUTH_TRAFFIC_SIGN_SENSOR,

  // Default sensors are initially the same as the ground truth sensors
  DEFAULT_EGO_SENSOR,
  DEFAULT_POWERTRAIN_SENSOR,
  DEFAULT_BRAKE_SENSOR,
  DEFAULT_WHEEL_SENSOR,
  DEFAULT_STEERING_SENSOR,
  DEFAULT_WORLD_SENSOR,
  DEFAULT_LANE_SENSOR,
  DEFAULT_TRAFFIC_SIGN_SENSOR,

  // Actuators are split into longitudinal and lateral controls
  // Groundtruth actuators should never be modified/replaced.
  GROUNDTRUTH_LONG_ACTUATOR,
  GROUNDTRUTH_LAT_ACTUATOR,
  GROUNDTRUTH_LATLONG_ACTUATOR,

  DEFAULT_LONG_ACTUATOR,
  DEFAULT_LAT_ACTUATOR,
  DEFAULT_LATLONG_ACTUATOR,
  DEFAULT_GEARBOX_ACTUATOR,
  DEFAULT_PEDAL_ACTUATOR,
  DEFAULT_STEERING_ACTUATOR,
};

// clang-format off
ENUM_SERIALIZATION(CloeComponent, ({
  // Groundtruth sensors
  {CloeComponent::GROUNDTRUTH_EGO_SENSOR, "cloe::gndtruth_ego_sensor"},
  {CloeComponent::GROUNDTRUTH_POWERTRAIN_SENSOR, "cloe::gndtruth_powertrain_sensor"},
  {CloeComponent::GROUNDTRUTH_BRAKE_SENSOR, "cloe::gndtruth_brake_sensor"},
  {CloeComponent::GROUNDTRUTH_WHEEL_SENSOR, "cloe::gndtruth_wheel_sensor"},
  {CloeComponent::GROUNDTRUTH_STEERING_SENSOR, "cloe::gndtruth_steering_sensor"},
  {CloeComponent::GROUNDTRUTH_WORLD_SENSOR, "cloe::gndtruth_world_sensor"},
  {CloeComponent::GROUNDTRUTH_LANE_SENSOR, "cloe::gndtruth_lane_sensor"},

  // Default sensors
  {CloeComponent::DEFAULT_EGO_SENSOR, "cloe::default_ego_sensor"},
  {CloeComponent::DEFAULT_POWERTRAIN_SENSOR, "cloe::default_powertrain_sensor"},
  {CloeComponent::DEFAULT_BRAKE_SENSOR, "cloe::default_brake_sensor"},
  {CloeComponent::DEFAULT_WHEEL_SENSOR, "cloe::default_wheel_sensor"},
  {CloeComponent::DEFAULT_STEERING_SENSOR, "cloe::default_steering_sensor"},
  {CloeComponent::DEFAULT_WORLD_SENSOR, "cloe::default_world_sensor"},
  {CloeComponent::DEFAULT_LANE_SENSOR, "cloe::default_lane_sensor"},

  // Groundturht actuators
  {CloeComponent::GROUNDTRUTH_LONG_ACTUATOR, "cloe::gndtruth_long_actuator"},
  {CloeComponent::GROUNDTRUTH_LAT_ACTUATOR, "cloe::gndtruth_lat_actuator"},
  {CloeComponent::GROUNDTRUTH_LATLONG_ACTUATOR, "cloe::gndtruth_latlong_actuator"},

  // Default actuators
  {CloeComponent::DEFAULT_LONG_ACTUATOR, "cloe::default_long_actuator"},
  {CloeComponent::DEFAULT_LAT_ACTUATOR, "cloe::default_lat_actuator"},
  {CloeComponent::DEFAULT_LATLONG_ACTUATOR, "cloe::default_latlong_actuator"},
  {CloeComponent::DEFAULT_GEARBOX_ACTUATOR, "cloe::default_gearbox_actuator"},
  {CloeComponent::DEFAULT_PEDAL_ACTUATOR, "cloe::default_pedal_actuator"},
  {CloeComponent::DEFAULT_STEERING_ACTUATOR, "cloe::default_steering_actuator"},
}))
// clang-format on

// to_string is necessary in order to be used by Vehicle's templated methods
inline std::string to_string(CloeComponent c) { return enum_serialization(c).at(c); }

}  // namespace cloe
