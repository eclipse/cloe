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
 * \file vtd_sensor_components.hpp
 */

#include <cloe/component/ego_sensor.hpp>     // for EgoSensor
#include <cloe/component/lane_sensor.hpp>    // for LaneBoundarySensor
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor

#include "task_control.hpp"     // for TaskControl
#include "vtd_sensor_data.hpp"  // for VtdSensorData

namespace vtd {

class VtdEgoSensor : public cloe::EgoSensor {
 public:
  VtdEgoSensor(uint64_t id, std::shared_ptr<VtdSensorData> data,
               std::shared_ptr<TaskControl> task_control)
      : EgoSensor("vtd/ego_sensor"), id_(id), data_{data}, task_control_{task_control} {}
  virtual ~VtdEgoSensor() = default;

  const cloe::Object& sensed_state() const override { return data_->get_ego_object(); }
  double wheel_steering_angle() const override { return data_->get_ego_steering_angle(); }
  double driver_request_acceleration() const override {
    return task_control_->get_driver_request_acceleration(id_);
  }
  double driver_request_wheel_steering_angle() const override {
    return task_control_->get_driver_request_steering_angle(id_);
  }
  virtual double steering_wheel_speed() const override {
    return task_control_->get_steering_wheel_speed(id_);
  }

 private:
  uint64_t id_;
  std::shared_ptr<VtdSensorData> data_;
  std::shared_ptr<TaskControl> task_control_;
};

class VtdWorldSensor : public cloe::ObjectSensor {
 public:
  explicit VtdWorldSensor(std::shared_ptr<VtdSensorData> data)
      : ObjectSensor("vtd/world_sensor"), data_{data} {}
  virtual ~VtdWorldSensor() = default;

  const cloe::Objects& sensed_objects() const override { return data_->get_world_objects(); }
  const cloe::Frustum& frustum() const override { return data_->get_frustum(); }
  const Eigen::Isometry3d& mount_pose() const override { return data_->get_mount_pose(); }

 private:
  std::shared_ptr<VtdSensorData> data_;
};

class VtdLaneBoundarySensor : public cloe::LaneBoundarySensor {
 public:
  explicit VtdLaneBoundarySensor(std::shared_ptr<VtdSensorData> data)
      : LaneBoundarySensor("vtd/lane_boundary_sensor"), data_{data} {}
  virtual ~VtdLaneBoundarySensor() = default;

  const cloe::LaneBoundaries& sensed_lane_boundaries() const override {
    return data_->get_lane_boundaries();
  }
  const cloe::Frustum& frustum() const override { return data_->get_frustum(); }
  const Eigen::Isometry3d& mount_pose() const override { return data_->get_mount_pose(); }

 private:
  std::shared_ptr<VtdSensorData> data_;
  std::shared_ptr<TaskControl> task_control_;
};

}  // namespace vtd
