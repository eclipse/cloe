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
 * \file esmini_osi_sensor.hpp
 * \see  esmini_osi_sensor.cpp
 */

#pragma once

#include <memory>  // for shared_ptr<>, unique_ptr<>
#include <string>  // for string, to_string

#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <esminiLib.hpp>  // for SE_UpdateOSIGroundTruth, ..

#include <cloe/component/object.hpp>             // for Object
#include <cloe/utility/osi_message_handler.hpp>  // for OsiMsgHandler
#include <cloe/utility/osi_transceiver.hpp>      // for OsiTransceiver

#include "esmini_logger.hpp"
#include "esmini_world_data.hpp"  // for ESMiniEnvData

namespace cloe_esmini {

class ESMiniOsiReceiver : public cloe::utility::OsiTransceiver {
 public:
  ~ESMiniOsiReceiver() override = default;

  /**
   * Update the ESMini osi::GroundTruth object and check return code.
   *
   * Note that SE_ClearOSIGroundTruth is called in clear_cache().
   */
  bool has_ground_truth() const override {
    int ierr = 0;
    if (update_static_ground_truth_) {
      ierr += SE_UpdateOSIStaticGroundTruth();
      update_static_ground_truth_ = false;
    }
    // Do not add the driver model's ghost vehicle to the object list.
    ierr += SE_UpdateOSIDynamicGroundTruth(/*reportGhost=*/false);
    return ierr == 0;
  }

  /**
   * ESMini does currently not provide osi::SensorView.
   */
  bool has_sensor_view() const override { return false; }

  /**
   * ESMini does currently not provide osi::SensorData.
   */
  bool has_sensor_data() const override { return false; }

  /**
   * ESMini does currently not provide osi::SensorView.
   */
  void receive_osi_msgs(std::vector<std::shared_ptr<osi3::SensorView>>& /*msgs*/) override {
    throw cloe::ModelError("ESMiniOsiReceiver: SensorView is currently not supported.");
  }

  /**
   * Fetch sensor model output from ESMini, if applicable.
   */
  void receive_osi_msgs(std::vector<std::shared_ptr<osi3::SensorData>>& msgs) override {
    if (!msgs.empty()) {
      esmini_logger()->warn(
          "ESMiniOsiReceiver: Non-zero length of message vector before retrieval: {}", msgs.size());
    }
    if (this->has_sensor_data()) {
      const auto* sd = reinterpret_cast<const osi3::SensorData*>(SE_GetOSISensorDataRaw());
      if (!sd->has_timestamp()) {
        throw cloe::ModelError("ESMiniOsiSensor: No timestamp in SensorData.");
      }
      msgs.push_back(std::make_shared<osi3::SensorData>(*sd));
    }
  }

  /**
   * Fetch ground truth from ESMini, if applicable.
   */
  void receive_osi_msgs(std::vector<std::shared_ptr<osi3::GroundTruth>>& msgs) override {
    if (!msgs.empty()) {
      esmini_logger()->warn(
          "ESMiniOsiReceiver: Non-zero length of message vector before retrieval: {}", msgs.size());
    }
    if (this->has_ground_truth()) {
      // NOTE: Make a copy because we are going to change it.
      auto gt = *reinterpret_cast<const osi3::GroundTruth*>(SE_GetOSIGroundTruthRaw());
      if (!gt.has_timestamp()) {
        throw cloe::ModelError("ESMiniOsiSensor: No timestamp in GroundTruth.");
      }
      msgs.push_back(std::make_shared<osi3::GroundTruth>(std::move(gt)));
    }
  }

  void clear_cache() override {
    // In ESMini v2.20.10, the SE_ClearOSIGroundTruth() was found
    // to vanish the gt->lane_boundary_ list (gt->lane_boundary_size()==0 after
    // first time step or first SE_ClearOSIGroundTruth() invocation).
    // Note that in their OSI coding example, they do not clear the cache:
    //   EnvironmentSimulator/code-examples/osi-groundtruth/osi-groundtruth.cpp
  }

  void to_json(cloe::Json& j) const override {
    j = cloe::Json{
        {"has_sensor_data", has_sensor_data()},
    };
  }

 private:
  mutable bool update_static_ground_truth_{true};
};

/**
 * ESMiniOsiSensor implements retrieval of all ground truth data provided by the simulator and conversion to the Cloe
 * sensor components.
 *
 * Note: Object and lane boundary data is converted to a fictive sensor position located in the vehicle reference point.
 *
 */
class ESMiniOsiSensor : public cloe::utility::OsiMsgHandler, public ESMiniEnvData {
 public:
  ~ESMiniOsiSensor() override = default;

  ESMiniOsiSensor(uint64_t owner_id, double filter_dist)
      : OsiMsgHandler(new ESMiniOsiReceiver(), owner_id), ESMiniEnvData("osi_sensor", filter_dist) {
    ego_object_ = std::make_shared<cloe::Object>();  // NOLINT
  }

  void step(const cloe::Sync& s) override {
    ESMiniEnvData::clear_cache();
    OsiMsgHandler::process_osi_msgs<osi3::GroundTruth>(s, restart_, env_data_time_);
    if (abs(env_data_time_.count() - env_data_time_next_.count()) >= s.step_width().count() / 100) {
      // Environment data time deviates from expected time by more than 1% of the time step.
      throw cloe::ModelError(
          "ESMiniOsiSensor: ESMini data at wrong timestamp. Expected: {}. Actual: {}.",
          env_data_time_next_.count(), env_data_time_.count());
    }
    env_data_time_next_ = s.time();
  }

  void store_object(std::shared_ptr<cloe::Object> obj) override { world_objects_.push_back(obj); }

  void store_lane_boundary(const cloe::LaneBoundary& lb) override { lanes_[lb.id] = lb; }

  void store_ego_object(std::shared_ptr<cloe::Object> ego_obj) override { ego_object_ = ego_obj; }

  void store_sensor_meta_data(const Eigen::Vector3d& bbcenter_to_veh_origin,
                              const Eigen::Vector3d& ego_dimensions) override {
    // Mounting position is not provided by ESMini -> nothing to do.
    (void)bbcenter_to_veh_origin;
    (void)ego_dimensions;
  }

  /**
   * Return the sensor pose in the vehicle reference frame as defined by OSI
   * (rear axle center, _not_ street level as in VTD).
   */
  Eigen::Isometry3d get_static_mounting_position(const Eigen::Vector3d& bbcenter_to_veh_origin,
                                                 const Eigen::Vector3d& ego_dimensions) override {
    Eigen::Isometry3d mount_osi = mount_;  // Identity (sensor mountin in Cloe ego vehicle origin)
    // Correct for the difference in reference frame z-location:
    //  Cloe/VTD: ground level; OSI: rear axle center.
    mount_osi.translation().z() =
        mount_osi.translation().z() - (0.5 * ego_dimensions(2) + bbcenter_to_veh_origin(2));
    return mount_osi;
  }

  /**
   * Set the mock level for different data types according to user request.
   */
  void set_mock_conf(std::shared_ptr<const cloe::utility::SensorMockConf> mock) override {
    this->mock_ = mock;
  }

  // As defined in `cloe/component.hpp`
  void reset() override {
    ESMiniEnvData::clear_cache();
    this->set_reset_state();
  }

  friend void to_json(cloe::Json& j, const ESMiniOsiSensor& s) {
    to_json(j, static_cast<const ESMiniEnvData&>(s));
    j = cloe::Json{
        {"osi_connection", s.osi_comm_},
    };
  }
};

}  // namespace cloe_esmini
