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
 * \file osi_sensor_component.hpp
 * \see  osi_sensor_component.cpp
 */

#pragma once

#include <memory>  // for shared_ptr<>, unique_ptr<>
#include <string>  // for string, to_string

#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <cloe/component/object.hpp>  // for Object
#include <cloe/utility/geometry.hpp>  // for quaternion_from_rpy

#include <osi/utility/osi_omni_sensor.hpp>  // for OsiOmniSensor
#include <osi/utility/osi_transceiver.hpp>  // for OsiTransceiver

#include "vtd_conf.hpp"         // for VtdSensorConfig
#include "vtd_sensor_data.hpp"  // for VtdSensorData

namespace vtd {

/**
 * VtdOsiSensor implements retrieval of all data sent by the simulator components.
 *
 * The object sensor senses box-like objects. The objects are received via TCP
 * and provided as an object list.
 *
 */
class VtdOsiSensor : public osii::OsiOmniSensor, public VtdSensorData {
 public:
  virtual ~VtdOsiSensor() = default;

  VtdOsiSensor(std::unique_ptr<osii::OsiTransceiver>&& osi_transceiver, uint64_t owner_id)
      : OsiOmniSensor(std::move(osi_transceiver), owner_id), VtdSensorData("osi_sensor") {
    ego_object_ = std::make_shared<cloe::Object>();  // NOLINT
  }

  void configure(const VtdSensorConfig& cfg);

  void step(const cloe::Sync& s) override {
    VtdSensorData::clear_cache();
    OsiOmniSensor::step_sensor_data(s, restart_, simulation_time_);
  }

  void store_object(std::shared_ptr<cloe::Object> obj) override { world_objects_.push_back(obj); }

  void store_lane_boundary(const cloe::LaneBoundary& lb) override { lanes_[lb.id] = lb; }

  void store_ego_object(std::shared_ptr<cloe::Object> ego_obj) override { ego_object_ = ego_obj; }

  void store_sensor_meta_data(const Eigen::Vector3d& bbcenter_to_veh_origin,
                              const Eigen::Vector3d& ego_dimensions) override {
    mount_ = osi_sensor_pose_;
    Eigen::Vector3d translation = osi_sensor_pose_.translation();
    // Correct for the difference in reference frame z-location.
    translation(2) = translation(2) + (0.5 * ego_dimensions(2) + bbcenter_to_veh_origin(2));
    mount_.translation() = translation;
  }

  /**
   * Return the sensor pose in the vehicle reference frame as defined by OSI
   * (rear axle center, _not_ street level as in VTD).
   */
  Eigen::Isometry3d get_static_mounting_position(const Eigen::Vector3d& bbcenter_to_veh_origin,
                                                 const Eigen::Vector3d& ego_dimensions) override {
    // VTD2.2: rotation order: "dhDeg (z-axis), dpDeg (y*-axis) and drDeg
    // (x**-axis). Each rotation is performed in the system resulting from the
    // previous rotation."
    // OSI3 rotation order: "yaw first (around the z-axis), pitch second (around
    // the new y-axis) and roll third (around the new x-axis)"
    Eigen::Quaterniond quaternion = cloe::utility::quaternion_from_rpy(
        vtd_mnt_ori_drpy_(0), vtd_mnt_ori_drpy_(1), vtd_mnt_ori_drpy_(2));
    Eigen::Vector3d translation = vtd_mnt_pos_dxyz_;
    // Correct for the difference in reference frame z-location:
    //  VTD: ground level; OSI: rear axle center.
    translation(2) = translation(2) - (0.5 * ego_dimensions(2) + bbcenter_to_veh_origin(2));
    return cloe::utility::pose_from_rotation_translation(quaternion, translation);
  }

  /**
   * Set the mock level for different data types according to user request.
   */
  void set_mock_conf(std::shared_ptr<const osii::SensorMockConf> mock) override {
    this->mock_ = mock;
  }

  // As defined in `cloe/component.hpp`
  void reset() override {
    VtdSensorData::clear_cache();
    this->set_reset_state();
  }

  friend void to_json(cloe::Json& j, const VtdOsiSensor& s) {
    to_json(j, static_cast<const VtdSensorData&>(s));
    j = cloe::Json{
        {"osi_connection", s.osi_comm_},
    };
  }

 protected:
  /// Sensor mounting position obtained from config (in VTD vehicle coordinate frame).
  Eigen::Vector3d vtd_mnt_pos_dxyz_;  // [m]
  Eigen::Vector3d vtd_mnt_ori_drpy_;  // [rad]
};

}  // namespace vtd
