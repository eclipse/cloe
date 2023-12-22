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
 * \file vtd_sensor_data.hpp
 * \see  omni_sensor_component.hpp
 * \see  osi_sensor_component.hpp
 * \see  task_control.hpp
 */

#pragma once

#include <memory>  // for shared_ptr<>, unique_ptr<>
#include <string>  // for string

#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/object.hpp>         // for Object
#include <cloe/sync.hpp>                     // for Sync

#include "vtd_logger.hpp"  // for vtd_logger

namespace vtd {

class VtdSensorData {
 public:
  VtdSensorData() = delete;
  virtual ~VtdSensorData() = default;

  /**
   * Construct a new instance of VtdSensorData with the given name.
   */
  explicit VtdSensorData(const std::string& name) : name_(name) {}

  /**
   * Process the incoming data.
   */
  virtual void step(const cloe::Sync& s) = 0;

  /**
   * Return the simulation time of the last processed frame.
   */
  virtual cloe::Duration time() const { return sensor_data_time_; }

  /**
   * Set the name of the sensor.
   *
   * The name is mainly used to improve readability of trace output so setting it is optional.
   */
  virtual void set_name(const std::string& name) { name_ = name; }

  /**
   * Reset the codec.
   *
   * Discard received messages, clear data members, and implement sensor-specific reset steps.
   */
  virtual void reset() = 0;

  /**
   * Notify the codec that it should reset.
   *
   * All messages with frame counter > 0 will be discarded in process() function
   * calls.
   */
  virtual void set_reset_state() { restart_ = true; }

  // Accessors
  const cloe::Object& get_ego_object() const { return *ego_object_; }

  const cloe::Objects& get_world_objects() const { return world_objects_; }

  double get_ego_steering_angle() const { return ego_steering_angle_; }

  const cloe::Frustum& get_frustum() const { return frustum_; }

  const Eigen::Isometry3d& get_mount_pose() const { return mount_; }

  const cloe::LaneBoundaries& get_lane_boundaries() { return lanes_; }

  // As defined in `cloe/component.hpp`
  void clear_cache() {
    world_objects_.clear();
    ego_object_ = std::make_shared<cloe::Object>();
    ego_steering_angle_ = 0.0;
    lanes_.clear();
  }

  friend void to_json(cloe::Json& j, const VtdSensorData& s) {
    j = cloe::Json{
        {"simulation_time", s.sensor_data_time_},      {"restart", s.restart_},
        {"world_objects", s.world_objects_},           {"ego_object", s.ego_object_},
        {"ego_steering_angle", s.ego_steering_angle_}, {"lane_boundaries", s.lanes_},
    };
  }

 protected:
  /// Human readable name.
  std::string name_ = "default_sensor";

  /// Indicates whether reset has been requested.
  bool restart_ = false;

  /// Simulation time from last processed sensor message.
  cloe::Duration sensor_data_time_ = cloe::Duration(0);

  /// Expected simulation time for next sensor message.
  cloe::Duration sensor_data_time_next_ = cloe::Duration(0);

  /// Sensor mounting position and orientation.
  Eigen::Isometry3d mount_;

  /// Sensor frustum information.
  cloe::Frustum frustum_;

  /// World objects from last processed frame.
  cloe::Objects world_objects_;

  /// ego object information from last processed frame.
  std::shared_ptr<cloe::Object> ego_object_;

  /// Ego front left wheel steering angle from last processed frame.
  double ego_steering_angle_{0.0};

  /// Lane id-to-boundary-map.
  cloe::LaneBoundaries lanes_;

  /// Traffic signs id-to-signs-map.
};

}  // namespace vtd
