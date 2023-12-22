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
 * \file osi_message_handler.hpp
 * \see  osi_message_handler.cpp
 */

#pragma once

#include <map>      // for map<>
#include <memory>   // for shared_ptr<>, unique_ptr<>
#include <string>   // for string
#include <utility>  // for move

#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/object.hpp>         // for Object
#include <cloe/core.hpp>                     // for Json, Duration
#include <cloe/simulator.hpp>                // for ModelError
#include <cloe/sync.hpp>                     // for Sync

#include "osi_common.pb.h"           // for Timestamp, Identifier, BaseMoving, ..
#include "osi_detectedobject.pb.h"   // for DetectedMovingObject
#include "osi_hostvehicledata.pb.h"  // for HostVehicleData
#include "osi_object.pb.h"           // for MovingObject
#include "osi_sensordata.pb.h"       // for SensorData, DetectedEntityHeader

#include "osi/utility/osi_ground_truth.hpp"  // for OsiGroundTruth
#include "osi/utility/osi_transceiver.hpp"   // for OsiTransceiver
#include "osi/utility/osi_utils.hpp"

namespace cloeosi {

/**
 * Convert OSI timestamp to Cloe time format.
 */
cloe::Duration osi_timestamp_to_time(const osi3::Timestamp& timestamp);

/**
 * OSI host vehicle coordinates/orientations are relative to the global ground
 * truth coordinate system. Here, this data is stored in a Cloe object.
 */
void from_osi_host_vehicle_data(const osi3::HostVehicleData& osi_hv, cloe::Object& obj);

/**
 * Map the OSI data fields without taking care of transformations to the Cloe
 * reference frame convention.
 * Note that the OSI reference frame may differ for different object data types.
 */
void from_osi_base_moving(const osi3::BaseMoving& osi_bm, cloe::Object& obj);

/**
 * As from_osi_base_moving, but use ground truth information if required data is
 * not provided by sensor (model).
 */
void from_osi_base_moving_alt(const osi3::BaseMoving& osi_bm,
                              const osi3::BaseMoving& osi_bm_gt,
                              cloe::Object& obj);

/**
 * As from_osi_base_moving, but for stationary objects.
 */
void from_osi_base_stationary(const osi3::BaseStationary& osi_bs, cloe::Object& obj);

template <typename T>
void from_osi_mov_obj_type_classification(const T& osi_mo, cloe::Object::Class& oc);
void from_osi_mov_obj_type_classification(
    const osi3::MovingObject::Type& osi_ot,
    const osi3::MovingObject::VehicleClassification::Type& osi_vt,
    cloe::Object::Class& oc);

void from_osi_detected_moving_object_alt(const osi3::DetectedMovingObject& osi_mo,
                                         const OsiGroundTruth& ground_truth, cloe::Object& obj);

void transform_ego_coord_from_osi_data(const Eigen::Vector3d& dimensions_gt, cloe::Object& obj);

/**
 * \param sensor_pose: Relation between the sensor frame and the ego vehicle
 *                     frame, expressed in the ego vehicle frame.
 * \param obj When enter: Geometric information in ego vehicle frame.
 *            When return: Geometric information in sensor frame.
 */
void transform_obj_coord_from_osi_data(const Eigen::Isometry3d& sensor_pose,
                                       const Eigen::Vector3d& dimensions_gt,
                                       cloe::Object& obj);

Eigen::Isometry3d osi_position_orientation_to_pose_alt(const osi3::BaseMoving& base,
                                                       const osi3::BaseMoving& base_gt);

Eigen::Vector3d osi_vehicle_attrib_rear_offset_to_vector3d(
    const osi3::MovingObject::VehicleAttributes& osi_va);

/**
 * OSI messages of the listed data types may be overwritten by ground truth
 * information, if requested by the user.
 */
enum class SensorMockTarget {
  MountingPosition,
  DetectedMovingObject,
  DetectedStaticObject,
  DetectedLaneBoundary
};

/**
 * SensorMockLevel determines to which degree an OSI message of a certain data type
 * should be overwritten by ground truth information:
 *  - `Zero` means that the message is not altered (default behavior).
 *  - `MissingData` means that unavailable data fields are filled.
 *  - `All` means that the entire message is overwritten.
 */
enum class SensorMockLevel { OverwriteNone, InterpolateMissing, OverwriteAll };

// clang-format off
ENUM_SERIALIZATION(SensorMockLevel, ({
  {SensorMockLevel::OverwriteNone, "overwrite_none"},
  {SensorMockLevel::InterpolateMissing, "interpolate_missing"},
  {SensorMockLevel::OverwriteAll, "overwrite_all"},
}))
// clang-format on

/**
 * Configure sensor mock level.
 */
struct SensorMockConf : public cloe::Confable {
  using Target = SensorMockTarget;
  using Level = SensorMockLevel;

  SensorMockConf() = default;

  virtual ~SensorMockConf() noexcept = default;

  std::map<Target, Level> level = {{Target::MountingPosition, Level::OverwriteNone},
                                   {Target::DetectedMovingObject, Level::OverwriteNone},
                                   {Target::DetectedStaticObject, Level::OverwriteNone},
                                   {Target::DetectedLaneBoundary, Level::OverwriteNone}};

  CONFABLE_SCHEMA(SensorMockConf) {
    return fable::Schema{
        // clang-format off
        {"mounting_position", cloe::Schema(&level[Target::MountingPosition], "mock level for sensor mounting position")},
        {"detected_moving_objects", cloe::Schema(&level[Target::DetectedMovingObject], "mock level for detected moving objects")},
        {"detected_static_objects", cloe::Schema(&level[Target::DetectedStaticObject], "mock level for detected stationary objects")},
        {"detected_lane_boundaries", cloe::Schema(&level[Target::DetectedLaneBoundary], "mock level for detected lane boundaries")},
        // clang-format on
    };
  }

  void to_json(cloe::Json& j) const override {
    j["mounting_position"] = level.at(SensorMockTarget::MountingPosition);
    j["detected_moving_objects"] = level.at(SensorMockTarget::DetectedMovingObject);
    j["detected_static_objects"] = level.at(SensorMockTarget::DetectedStaticObject);
    j["detected_lane_boundaries"] = level.at(SensorMockTarget::DetectedLaneBoundary);
  }
};

/**
 * Base class for an OSI sensor which is connected via TCP.
 */
class OsiMsgHandler {
 public:
  virtual ~OsiMsgHandler() = default;
  OsiMsgHandler() = delete;

  /**
   * Create a new instance of OsiMsgHandler with the given OsiTransceiver.
   */
  OsiMsgHandler(std::unique_ptr<OsiTransceiver>&& osi_transceiver, uint64_t owner_id)
      : osi_comm_(std::move(osi_transceiver)), owner_id_(owner_id) {
    ground_truth_ = std::make_unique<OsiGroundTruth>();
  }

  /**
   * Create a new instance of OsiMsgHandler with the given new OsiTransceiver.
   *
   * WARNING: If you use this constructor, please realize that OsiMsgHandler
   *          takes ownership of the pointer you pass in.
   */
  OsiMsgHandler(OsiTransceiver* osi_transceiver, uint64_t owner_id)
      : osi_comm_(osi_transceiver), owner_id_(owner_id) {
    ground_truth_ = std::make_unique<OsiGroundTruth>();
  }

  /**
   * Receive and process the incoming osi3 messages.
   */
  template <typename T>
  void process_osi_msgs(const cloe::Sync& s, const bool& restart, cloe::Duration& osi_time);

  /**
   * Store the initial timestamp.
   * Note that the osi time does not necessarily start at zero.
   */
  void handle_first_message(const osi3::Timestamp& timestamp);

  /**
   * Translate OSI SensorData to Cloe data objects.
   *
   * \param osi_sd SensorData message to be processed.
   * \param osi_time Timestamp of the OSI message.
   */
  virtual void process_received_msg(osi3::SensorData* osi_sd, cloe::Duration& osi_time);

  /**
   * Translate OSI SensorView to Cloe data objects.
   *
   * \param osi_sv SensorView message to be processed.
   * \param osi_time Timestamp of the OSI message.
   */
  virtual void process_received_msg(osi3::SensorView* osi_sv, cloe::Duration& osi_time);

  /**
   * Translate OSI GroundTruth to Cloe data objects.
   *
   * \param osi_gt GroundTruth message to be processed.
   * \param osi_time Timestamp of the OSI message.
   */
  virtual void process_received_msg(osi3::GroundTruth* osi_gt, cloe::Duration& osi_time);

  /**
   * Translate OSI GroundTruth to Cloe data objects.
   *
   * \param osi_gt GroundTruth message to be processed.
   */
  virtual void convert_to_cloe_data(const osi3::GroundTruth& osi_gt);

  /**
   * Translate OSI SensorView to Cloe data objects.
   *
   * \param osi_sv SensorView message to be processed, including ground truth.
   */
  virtual void convert_to_cloe_data(const osi3::SensorView& osi_sv);

  /**
   * Translate OSI ego base information made available to the sensor model
   * from other components, or use ground truth.
   * \param osi_mo MovingObject (ground truth) used as fallback.
   * \param osi_hv Pointer to HostVehicleData message to be processed (if available).
   */
  virtual void convert_to_cloe_data(const osi3::MovingObject& osi_ego,
                                    const osi3::HostVehicleData* osi_hv);

  /**
   * Translate OSI detected moving object information to Cloe data objects.
   *
   * \param osi_eh DetectedEntityHeader message to be processed (if available).
   * \param osi_mo DetectedMovingObject message to be processed.
   */
  virtual void convert_to_cloe_data(const bool has_eh,
                                    const osi3::DetectedEntityHeader& osi_eh,
                                    const osi3::DetectedMovingObject& osi_mo);

  void detected_moving_objects_from_ground_truth();

  void detected_static_objects_from_ground_truth();

  void detected_lane_boundaries_from_ground_truth();

  void from_osi_boundary_points(const osi3::LaneBoundary& osi_lb, cloe::LaneBoundary& lb,
                                bool reverse_pt_order);

  /**
   * Store the ego object that should be passed to Cloe.
   *
   * \param ego_obj Ego object to be stored.
   */
  virtual void store_ego_object(std::shared_ptr<cloe::Object> ego_obj) = 0;

  /**
   * Store a detected object in a list of Cloe data objects.
   *
   * \param obj Object to be stored.
   */
  virtual void store_object(std::shared_ptr<cloe::Object> obj) = 0;

  /**
   * Store a detected lane boundary in a map of Cloe data objects.
   *
   * \param lb Lane boundary to be stored.
   */
  virtual void store_lane_boundary(const cloe::LaneBoundary& lb) = 0;

  /**
   * Store the sensor pose etc. in the corresponding Cloe sensor component.
   */
  virtual void store_sensor_meta_data(const Eigen::Vector3d& bbcenter_to_veh_origin,
                                      const Eigen::Vector3d& ego_dimensions) = 0;

  /**
  * Get the current simulation time (t-t0).
  */
  cloe::Duration osi_timestamp_to_simtime(const osi3::Timestamp& timestamp) const;

  /**
  * Get sensor pose in OSI vehicle reference frame, e.g. from simulator configuration.
  */
  virtual Eigen::Isometry3d get_static_mounting_position(
      const Eigen::Vector3d& bbcenter_to_veh_origin, const Eigen::Vector3d& ego_dimensions) = 0;

  virtual void set_mock_conf(std::shared_ptr<const SensorMockConf> mock) = 0;

  SensorMockLevel get_mock_level(SensorMockTarget trg_type) const {
    return mock_->level.at(trg_type);
  }

  /**
  * Clear sensor and transceiver cache, if applicable.
  */
  virtual void clear_cache() { osi_comm_->clear_cache(); }

  friend void to_json(cloe::Json& j, const OsiMsgHandler& c) {
    j = cloe::Json{
        {"osi_connection", *c.osi_comm_},
    };
  }

 protected:
  /// Connection via TCP to simulator.
  /// Should always be valid.
  std::unique_ptr<OsiTransceiver> osi_comm_;

  /// Access to osi ground truth, e.g. for mock-ups.
  std::unique_ptr<OsiGroundTruth> ground_truth_;

  /// Id of the sensor's owner (ego).
  uint64_t owner_id_;

  /// Store ego pose (reference point is rear axle center, not ground level).
  Eigen::Isometry3d osi_ego_pose_;

  /// Store sensor pose relative to the ego frame (rear axle center, not ground level).
  Eigen::Isometry3d osi_sensor_pose_;

  /// Initial simulation time.
  cloe::Duration init_time_ = cloe::Duration(-1);

  /// Use alternative source for required data or overwrite incoming data, if requested.
  std::shared_ptr<const SensorMockConf> mock_{nullptr};
};
}  // namespace cloeosi
