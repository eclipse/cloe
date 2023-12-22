/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file esmini_sensor_components.hpp
 */

#include <cloe/component/ego_sensor.hpp>     // for EgoSensor
#include <cloe/component/lane_sensor.hpp>    // for LaneBoundarySensor
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor

#include "esmini_world_data.hpp"  // for ESMiniEnvData

namespace esmini {

class ESMiniEgoSensor : public cloe::EgoSensor {
 public:
  ESMiniEgoSensor(uint64_t id, std::shared_ptr<ESMiniEnvData> data)
      : EgoSensor("esmini/ego_sensor"), id_(id), env_data_{data} {}
  virtual ~ESMiniEgoSensor() = default;

  const cloe::Object& sensed_state() const override { return env_data_->get_ego_object(); }
  double wheel_steering_angle() const override { return env_data_->get_ego_steering_angle(); }
  virtual double steering_wheel_speed() const override {
    throw cloe::ModelError("ESMiniEgoSensor: steering wheel speed not available from ESMini.");
    return 0.0;
  }

 private:
  uint64_t id_;
  std::shared_ptr<ESMiniEnvData> env_data_;
};

class ESMiniObjectSensor : public cloe::ObjectSensor {
 public:
  explicit ESMiniObjectSensor(std::shared_ptr<ESMiniEnvData> data)
      : ObjectSensor("esmini/object_sensor"), env_data_{data} {}
  virtual ~ESMiniObjectSensor() = default;

  const cloe::Objects& sensed_objects() const override { return env_data_->get_world_objects(); }
  const cloe::Frustum& frustum() const override { return env_data_->get_frustum(); }
  const Eigen::Isometry3d& mount_pose() const override { return env_data_->get_mount_pose(); }

 private:
  std::shared_ptr<ESMiniEnvData> env_data_;
};

class ESMiniLaneBoundarySensor : public cloe::LaneBoundarySensor {
 public:
  explicit ESMiniLaneBoundarySensor(std::shared_ptr<ESMiniEnvData> data)
      : LaneBoundarySensor("esmini/lane_boundary_sensor"), env_data_{data} {}
  virtual ~ESMiniLaneBoundarySensor() = default;

  const cloe::LaneBoundaries& sensed_lane_boundaries() const override {
    return env_data_->get_lane_boundaries();
  }
  const cloe::Frustum& frustum() const override { return env_data_->get_frustum(); }
  const Eigen::Isometry3d& mount_pose() const override { return env_data_->get_mount_pose(); }

 private:
  std::shared_ptr<ESMiniEnvData> env_data_;
};

}  // namespace esmini
