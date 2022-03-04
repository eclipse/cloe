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
 * \file osi_omni_sensor.cpp
 * \see  osi_omni_sensor.hpp
 */

#include "osi_omni_sensor.hpp"

#include <math.h>     // for atan
#include <algorithm>  // for max
#include <cassert>    // for assert
#include <map>        // for map<>

#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/object.hpp>         // for Object
#include <cloe/core.hpp>                     // for Duration
#include <cloe/simulator.hpp>                // for ModelError
#include <cloe/utility/geometry.hpp>         // for quaternion_from_rpy

#include "osi_common.pb.h"           // for Timestamp, Identifier, BaseMoving, ..
#include "osi_detectedobject.pb.h"   // for DetectedMovingObject
#include "osi_hostvehicledata.pb.h"  // for HostVehicleData
#include "osi_object.pb.h"           // for MovingObject
#include "osi_sensordata.pb.h"       // for SensorData, DetectedEntityHeader
#include "osi_sensorview.pb.h"       // for SensorView

#include "osi_ground_truth.hpp"  // for OsiGroundTruth
#include "osi_utils.hpp"         // for osi_require, ..

namespace osii {

Eigen::Isometry3d osi_position_orientation_to_pose_alt(const osi3::BaseMoving& base,
                                                       const osi3::BaseMoving& base_gt) {
  const osi3::BaseMoving* base_p;
  if (base.has_orientation()) {
    base_p = &base;
  } else {
    osi_require("GroundTruth-BaseMoving::orientation", base_gt.has_orientation());
    base_p = &base_gt;
  }
  Eigen::Quaterniond quaternion = cloe::utility::quaternion_from_rpy(
      base_p->orientation().roll(), base_p->orientation().pitch(), base_p->orientation().yaw());

  osi_require("base::position", base.has_position());
  Eigen::Vector3d translation = osi_vector3d_xyz_to_vector3d(base.position());
  return cloe::utility::pose_from_rotation_translation(quaternion, translation);
}

/**
 * Convert from OSI moving object type to Cloe object classification. Note that
 * vehicles are treated explicitly in osi_mov_veh_class_map.
 */
const std::map<osi3::MovingObject::Type, cloe::Object::Class> osi_mov_obj_type_map = {
    {osi3::MovingObject_Type_TYPE_UNKNOWN, cloe::Object::Class::Unknown},
    {osi3::MovingObject_Type_TYPE_OTHER, cloe::Object::Class::Unknown},
    {osi3::MovingObject_Type_TYPE_ANIMAL, cloe::Object::Class::Unknown},
    {osi3::MovingObject_Type_TYPE_PEDESTRIAN, cloe::Object::Class::Pedestrian},
};

/**
 * Convert from OSI moving vehicle type to Cloe object classification. Note that
 * objects other than vehicles are treated explicitly in osi_mov_obj_type_map.
 */
const std::map<osi3::MovingObject::VehicleClassification::Type, cloe::Object::Class>
    osi_mov_veh_class_map = {
        {osi3::MovingObject_VehicleClassification_Type_TYPE_UNKNOWN, cloe::Object::Class::Unknown},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_OTHER, cloe::Object::Class::Unknown},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_SMALL_CAR, cloe::Object::Class::Car},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_COMPACT_CAR, cloe::Object::Class::Car},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_MEDIUM_CAR, cloe::Object::Class::Car},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_LUXURY_CAR, cloe::Object::Class::Car},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_DELIVERY_VAN,
         cloe::Object::Class::Truck},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_HEAVY_TRUCK,
         cloe::Object::Class::Truck},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_SEMITRAILER,
         cloe::Object::Class::Truck},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_TRAILER, cloe::Object::Class::Unknown},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_MOTORBIKE,
         cloe::Object::Class::Motorbike},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_BICYCLE, cloe::Object::Class::Bike},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_BUS, cloe::Object::Class::Truck},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_TRAM, cloe::Object::Class::Unknown},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_TRAIN, cloe::Object::Class::Unknown},
        {osi3::MovingObject_VehicleClassification_Type_TYPE_WHEELCHAIR,
         cloe::Object::Class::Unknown},
};

/**
 * Convert from OSI lane boundary types to Cloe types.
 */
const std::map<osi3::LaneBoundary_Classification_Type, cloe::LaneBoundary::Type>
    osi_lane_bdry_type_map = {
        // clang-format off
        {osi3::LaneBoundary_Classification_Type_TYPE_UNKNOWN, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_OTHER, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_NO_LINE, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_SOLID_LINE, cloe::LaneBoundary::Type::Solid},
        {osi3::LaneBoundary_Classification_Type_TYPE_DASHED_LINE, cloe::LaneBoundary::Type::Dashed},
        {osi3::LaneBoundary_Classification_Type_TYPE_BOTTS_DOTS, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_ROAD_EDGE, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_SNOW_EDGE, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_GRASS_EDGE, cloe::LaneBoundary::Type::Grass},
        {osi3::LaneBoundary_Classification_Type_TYPE_GRAVEL_EDGE, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_SOIL_EDGE, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_GUARD_RAIL, cloe::LaneBoundary::Type::Unknown},
        {osi3::LaneBoundary_Classification_Type_TYPE_CURB, cloe::LaneBoundary::Type::Curb},
        {osi3::LaneBoundary_Classification_Type_TYPE_STRUCTURE, cloe::LaneBoundary::Type::Unknown},
        // clang-format on
};

/**
 * Convert from OSI lane boundary colors to Cloe colors.
 */
const std::map<int, cloe::LaneBoundary::Color> osi_lane_bdry_color_map = {
    {osi3::LaneBoundary_Classification_Color_COLOR_UNKNOWN, cloe::LaneBoundary::Color::Unknown},
    {osi3::LaneBoundary_Classification_Color_COLOR_OTHER, cloe::LaneBoundary::Color::Unknown},
    {osi3::LaneBoundary_Classification_Color_COLOR_NONE, cloe::LaneBoundary::Color::Unknown},
    {osi3::LaneBoundary_Classification_Color_COLOR_WHITE, cloe::LaneBoundary::Color::White},
    {osi3::LaneBoundary_Classification_Color_COLOR_YELLOW, cloe::LaneBoundary::Color::Yellow},
    {osi3::LaneBoundary_Classification_Color_COLOR_RED, cloe::LaneBoundary::Color::Red},
    {osi3::LaneBoundary_Classification_Color_COLOR_BLUE, cloe::LaneBoundary::Color::Blue},
    {osi3::LaneBoundary_Classification_Color_COLOR_GREEN, cloe::LaneBoundary::Color::Green},
    {osi3::LaneBoundary_Classification_Color_COLOR_VIOLET, cloe::LaneBoundary::Color::Unknown},
};

cloe::Duration osi_timestamp_to_time(const osi3::Timestamp& timestamp) {
  return std::chrono::duration_cast<cloe::Duration>(std::chrono::seconds(timestamp.seconds()) +
                                                    std::chrono::nanoseconds(timestamp.nanos()));
}

cloe::Duration OsiOmniSensor::osi_timestamp_to_simtime(const osi3::Timestamp& timestamp) const {
  return osi_timestamp_to_time(timestamp) - this->init_time_;
}

void from_osi_identifier(const osi3::Identifier& osi_id, int& id) {
  id = static_cast<int>(osi_id.value());
}

void from_osi_host_vehicle_data(const osi3::HostVehicleData& osi_hv, cloe::Object& obj) {
  from_osi_base_moving(osi_hv.location(), obj);
}

void from_osi_detected_item_header(const osi3::DetectedItemHeader& osi_hdr, cloe::Object& obj) {
  osi_require("ground_truth_id_size == 1", osi_hdr.ground_truth_id_size() == 1);
  // Multiple ground truth objects melt into one detected item are currently not
  // supported.
  auto osi_obj_gt_id = osi_hdr.ground_truth_id(0);
  from_osi_identifier(osi_obj_gt_id, obj.id);
  // Existence probability
  if (osi_hdr.has_existence_probability()) {
    obj.exist_prob = osi_hdr.existence_probability();
  } else {
    obj.exist_prob = 1.0;
  }
}

void from_osi_detected_moving_object(const osi3::DetectedMovingObject& osi_mo, cloe::Object& obj) {
  // object id = ground truth id
  osi_require("DetectedMovingObject::header", osi_mo.has_header());
  from_osi_detected_item_header(osi_mo.header(), obj);

  // Object classification
  if (osi_mo.candidate_size() > 0) {
    osi_require("candidate_size == 1", osi_mo.candidate_size() == 1);
    from_osi_mov_obj_type_classification(osi_mo.candidate(0), obj.classification);
    // TODO(tobias): Need to additionally handle classification probability.
  } else {
    obj.classification = cloe::Object::Class::Unknown;
  }

  // DetectedMovingObject::base: "The bounding box does NOT include mirrors for
  // vehicles. The parent frame of base is the sensor's [vehicle frame]."
  from_osi_base_moving(osi_mo.base(), obj);
  // TODO(tobias): handle sensor-specific data: if (osi_mo.has_radar_specifics())
}

void from_osi_detected_moving_object_alt(const osi3::DetectedMovingObject& osi_mo,
                                         const OsiGroundTruth& ground_truth,
                                         cloe::Object& obj) {
  // Object id = ground truth id
  osi_require("DetectedMovingObject::header", osi_mo.has_header());
  from_osi_detected_item_header(osi_mo.header(), obj);

  // Get ground truth info for this object as fallback for missing data.
  osi3::MovingObject osi_mo_gt(*(ground_truth.get_moving_object(obj.id)));
  osi3::MovingObject osi_ego_gt(*(ground_truth.get_moving_object(ground_truth.get_ego_id())));
  // Transform coordinates to osi detected object convention, i.e. into ego
  // vehicle frame.
  osi_transform_base_moving(osi_ego_gt.base(), *(osi_mo_gt.mutable_base()));

  // Object classification
  if (osi_mo.candidate_size() > 0) {
    osi_require("candidate_size == 1", osi_mo.candidate_size() == 1);
    from_osi_mov_obj_type_classification(osi_mo.candidate(0), obj.classification);
    // TODO(tobias): Need to additionally handle classification probability.
  } else {
    from_osi_mov_obj_type_classification(osi_mo_gt, obj.classification);
  }

  assert(obj.id != static_cast<int>(ground_truth.get_ego_id()));
  // DetectedMovingObject::base: "The bounding box does NOT include mirrors for
  // vehicles. The parent frame of base is the sensor's [vehicle frame]."
  from_osi_base_moving_alt(osi_mo.base(), osi_mo_gt.base(), obj);
  // TODO(tobias): handle sensor-specific data: if (osi_mo.has_radar_specifics())
}

void from_osi_base_moving(const osi3::BaseMoving& osi_bm, cloe::Object& obj) {
  obj.type = cloe::Object::Type::Dynamic;

  obj.pose = osi_position_orientation_to_pose(osi_bm);

  osi_require("BaseMoving::dimension", osi_bm.has_dimension());
  obj.dimensions = osi_dimension3d_lwh_to_vector3d(osi_bm.dimension());

  osi_require("BaseMoving::acceleration", osi_bm.has_acceleration());
  obj.acceleration = osi_vector3d_xyz_to_vector3d(osi_bm.acceleration());

  osi_require("BaseMoving::velocity", osi_bm.has_velocity());
  obj.velocity = osi_vector3d_xyz_to_vector3d(osi_bm.velocity());

  osi_require("BaseMoving::orientation_rate", osi_bm.has_orientation_rate());
  obj.angular_velocity = osi_orientation3d_rpy_to_vector3d(osi_bm.orientation_rate());
}

void from_osi_base_moving_alt(const osi3::BaseMoving& osi_bm, const osi3::BaseMoving& osi_bm_gt,
                              cloe::Object& obj) {
  obj.type = cloe::Object::Type::Dynamic;

  obj.pose = osi_position_orientation_to_pose_alt(osi_bm, osi_bm_gt);

  assert(osi_bm.has_dimension());
  obj.dimensions = osi_dimension3d_lwh_to_vector3d(osi_bm.dimension());

  assert(osi_bm.has_acceleration());
  obj.acceleration = osi_vector3d_xyz_to_vector3d(osi_bm.acceleration());

  assert(osi_bm.has_velocity());
  obj.velocity = osi_vector3d_xyz_to_vector3d(osi_bm.velocity());

  if (osi_bm.has_orientation_rate()) {
    obj.angular_velocity = osi_orientation3d_rpy_to_vector3d(osi_bm.orientation_rate());
  } else {
    assert(osi_bm_gt.has_orientation_rate());
    obj.angular_velocity = osi_orientation3d_rpy_to_vector3d(osi_bm_gt.orientation_rate());
  }
}

template <typename T>
void from_osi_mov_obj_type_classification(const T& osi_mo, cloe::Object::Class& oc) {
  if (!osi_mo.has_type()) {
    throw cloe::ModelError("OSI missing moving object type");
  }

  if (osi_mo.type() == osi3::MovingObject_Type_TYPE_VEHICLE) {
    if (!osi_mo.has_vehicle_classification()) {
      throw cloe::ModelError("OSI missing moving vehicle classification");
    }
    if (!osi_mo.vehicle_classification().has_type()) {
      throw cloe::ModelError("OSI missing moving vehicle classification type");
    }
  }

  from_osi_mov_obj_type_classification(osi_mo.type(), osi_mo.vehicle_classification().type(), oc);
}

template void from_osi_mov_obj_type_classification<osi3::MovingObject>(
    const osi3::MovingObject& osi_mo, cloe::Object::Class& oc);
template void
from_osi_mov_obj_type_classification<osi3::DetectedMovingObject::CandidateMovingObject>(
    const osi3::DetectedMovingObject::CandidateMovingObject& osi_mo, cloe::Object::Class& oc);

void from_osi_mov_obj_type_classification(
    const osi3::MovingObject::Type& osi_ot,
    const osi3::MovingObject::VehicleClassification::Type& osi_vt,
    cloe::Object::Class& oc) {
  if (osi_ot == osi3::MovingObject_Type_TYPE_VEHICLE) {
    oc = osi_mov_veh_class_map.at(osi_vt);
  } else {
    oc = osi_mov_obj_type_map.at(osi_ot);
  }
}

void transform_ego_coord_from_osi_data(const Eigen::Vector3d& dimensions_gt, cloe::Object& obj) {
  // obj->pose: Change object position from bbox-center to vehicle reference
  // point (rear axle/street level):
  //  - Shift (x,y) to rear axis center using given osi bbcenter_to_rear vector.
  //  - Shift (z) to street level using bbox half-height.
  Eigen::Vector3d bbcenter_to_rear_street{obj.cog_offset(0), obj.cog_offset(1),
                                          -0.5 * dimensions_gt(2)};

  // Transform translation vector from vehicle frame into world frame.
  Eigen::Vector3d pos_veh_origin =
      obj.pose.translation() + obj.pose.rotation() * bbcenter_to_rear_street;

  obj.pose.translation() = pos_veh_origin;

  // cog is on street level, i.e. only x-offset is non-zero. Here, the direction
  // is opposite as defined in the OSI standard.
  obj.cog_offset = Eigen::Vector3d(-1.0 * obj.cog_offset(0), 0.0, 0.0);

  // Convert ego velocity and acceleration into ego vehicle frame coordinates.
  obj.velocity = obj.pose.rotation().inverse() * obj.velocity;
  obj.acceleration = obj.pose.rotation().inverse() * obj.acceleration;
}

void transform_obj_coord_from_osi_data(const Eigen::Isometry3d& sensor_pose,
                                       const Eigen::Vector3d& dimensions_gt, cloe::Object& obj) {
  // obj->pose/velocity/acceleration/angular_velocity:
  // Transform the location and orientation of the detected object from the ego
  // vehicle frame into the sensor reference frame.
  Eigen::Vector3d obj_pos_sensor_frame =
      sensor_pose.rotation().inverse() * (obj.pose.translation() - sensor_pose.translation());
  obj.pose.translation() = obj_pos_sensor_frame;
  obj.pose.rotate(sensor_pose.rotation().inverse());

  obj.velocity = sensor_pose.rotation().inverse() * obj.velocity;
  obj.acceleration = sensor_pose.rotation().inverse() * obj.acceleration;
  obj.angular_velocity = sensor_pose.rotation().inverse() * obj.angular_velocity;

  // obj->pose: Change the object position reference point from the bounding box
  // center to the vehicle reference point (rear axle/street level).
  Eigen::Vector3d bbcenter_to_rear_street{obj.cog_offset(0), obj.cog_offset(1),
                                          -0.5 * dimensions_gt(2)};

  // Transform translation vector from the object reference frame into the
  // sensor frame.
  obj_pos_sensor_frame = obj.pose.translation() + obj.pose.rotation() * bbcenter_to_rear_street;

  obj.pose.translation() = obj_pos_sensor_frame;

  // cog is on street level, i.e. only x-offset is non-zero. Here, the direction
  // is opposite as defined in the OSI standard.
  obj.cog_offset = Eigen::Vector3d(-1.0 * obj.cog_offset(0), 0.0, 0.0);
}

void OsiOmniSensor::step(const cloe::Sync& s, const bool& restart, cloe::Duration& sim_time) {
  // Cycle until sensor data has been received.
  int n_msg{0};
  while (n_msg == 0 || restart) {
    auto osi_msg = osi_comm_->receive_sensor_data();
    if (osi_msg.size() > 0) {
      osi_logger()->trace("OsiOmniSensor: processing {} messages at Cloe frame no {}",
                          osi_msg.size(), s.step());
      // 1st. timestep: Store the simulation reference (e.g. start) time.
      this->process(osi_msg[0]->timestamp());
    }
    for (auto m : osi_msg) {
      this->process(m.get(), sim_time);
      ++n_msg;
    }
  }

  if (abs(sim_time.count() - s.time().count()) >= s.step_width().count() / 100) {
    // Sensor data time deviates from cloe time by more than 1% of the time step.
    osi_logger()->warn("OsiOmniSensor: inconsistent timestamps [t_sensor={}ns, t_cloe={}ns]",
                       sim_time.count(), s.time().count());
  }

  osi_logger()->trace("OsiOmniSensor: completed processing messages [frame={}, time={}ns]",
                      s.step(), s.time().count());
}

void OsiOmniSensor::process(const osi3::Timestamp& timestamp) {
  // TODO(tobias): probably needs to be changed for restarts
  if (init_time_.count() >= 0.0) {
    return;
  }
  init_time_ = osi_timestamp_to_time(timestamp);
}

void OsiOmniSensor::process(osi3::SensorData* osi_sd, cloe::Duration& sim_time) {
  if (osi_sd == nullptr) {
    return;
  }

  if (osi_sd->ByteSize() == 0) {
    return;
  }

  osi_require("v3.x.x", !osi_sd->has_version() || osi_sd->version().version_major() > 2);

  // TODO(tobias): handle restart

  // Read the time when the message was sent, which is after capturing and
  // processing the sensor raw signal.
  if (osi_sd->has_timestamp()) {
    sim_time = osi_timestamp_to_simtime(osi_sd->timestamp());
    osi_logger()->trace("OsiOmniSensor: message @ {} ns", sim_time.count());
  } else {
    throw cloe::ModelError("OsiOmniSensor: No timestamp in SensorData. FMU properly loaded?");
  }

  // Read the time of the ground truth scene that was processed.
  if (osi_sd->has_last_measurement_time()) {
    cloe::Duration meas_time = osi_timestamp_to_simtime(osi_sd->last_measurement_time());
    osi_logger()->trace("OsiOmniSensor: measurement @ {} ns", meas_time.count());
  } else {
    osi_logger()->info("OsiOmniSensor: last_measurement_time not available in SensorData.");
  }

  // Obtain ego data from sensor views (sensor model input), i.e. ground truth.
  osi_require("SensorData::SensorView", osi_sd->sensor_view_size() > 0);
  const osi3::MountingPosition* mnt_pos{nullptr};
  for (int i_sv = 0; i_sv < osi_sd->sensor_view_size(); ++i_sv) {
    this->process(osi_sd->sensor_view(i_sv));
    if (osi_sd->sensor_view(i_sv).has_mounting_position()) {
      mnt_pos = &(osi_sd->sensor_view(i_sv).mounting_position());
    }
  }

  if (osi_sd->has_mounting_position()) {
    // Give higher priority to the sensor model output (SensorData) than to SensorView.
    mnt_pos = &(osi_sd->mounting_position());
  }

  // Store sensor mounting position and orientation for reference frame transformations.
  if (mnt_pos) {
    osi_sensor_pose_ = osi_position_orientation_to_pose(*mnt_pos);
  } else {
    if (this->get_mock_level(SensorMockTarget::MountingPosition) !=
        SensorMockLevel::OverwriteNone) {
      osi_sensor_pose_ =
          get_static_mounting_position(ground_truth_->get_veh_coord_sys_info(owner_id_),
                                       ground_truth_->get_mov_obj_dimensions(owner_id_));
    } else {
      throw cloe::ModelError("OSI sensor mounting position is not available");
    }
  }

  if (osi_sd->has_host_vehicle_location()) {
    // Sensor has its own estimate of the vehicle location, which we could use
    // to overwrite the ego pose that was taken from ground truth.
    throw cloe::ModelError("OSI host_vehicle_location handling is not yet available");
  }

  // Process detected moving objects.
  for (int i_mo = 0; i_mo < osi_sd->moving_object_size(); ++i_mo) {
    this->process(osi_sd->has_moving_object_header(), osi_sd->moving_object_header(),
                  osi_sd->moving_object(i_mo));
  }

  // TODO(tobias): Process detected stationary objects.

  // Process lane boundaries.
  switch (this->get_mock_level(SensorMockTarget::DetectedLaneBoundary)) {
    case SensorMockLevel::OverwriteAll: {
      mock_detected_lane_boundaries();
      break;
    }
    default: {
      //  TODO(tobias): Detected road marking handling is not yet available.
      break;
    }
  }

  // TODO(tobias): Process detected lanes once supported by Cloe data model.

  // TODO(tobias): Process detected traffic signs.

  // TODO(tobias): Process detected traffic lights once supported by Cloe data model.

  store_sensor_meta_data(ground_truth_->get_veh_coord_sys_info(owner_id_),
                         ground_truth_->get_mov_obj_dimensions(owner_id_));

  // Cleanup
  ground_truth_->reset();
}

void OsiOmniSensor::process(const osi3::SensorView& osi_sv) {
  if (osi_sv.ByteSize() == 0) {
    return;
  }

  // Fill the coordinate system info from ground truth.
  osi_require("SensorView::GroundTruth", osi_sv.has_global_ground_truth());
  const osi3::GroundTruth* osi_gt = &(osi_sv.global_ground_truth());
  ground_truth_->set(*osi_gt);

  for (int i_mo = 0; i_mo < osi_gt->moving_object_size(); ++i_mo) {
    osi3::MovingObject osi_mo = osi_gt->moving_object(i_mo);
    int obj_id;
    from_osi_identifier(osi_mo.id(), obj_id);

    // Store geometric information of different object reference frames.
    if (osi_mo.has_vehicle_attributes()) {
      ground_truth_->store_veh_coord_sys_info(obj_id, osi_mo.vehicle_attributes());
    }

    // Store object bounding box dimensions for cooordinate transformations.
    osi_require("GroundTruth::MovingObject::base", osi_mo.has_base());
    if (osi_mo.has_base()) {
      osi_require("GroundTruth-BaseMoving::dimension", osi_mo.base().has_dimension());
      ground_truth_->store_mov_obj_dimensions(obj_id, osi_mo.base().dimension());
    }
  }

  // Process ego vehicle info. For the ego, we may use ground truth information.
  // Note: osi.sv.host_vehicle_id() may not be populated.
  auto osi_ego = ground_truth_->get_moving_object(ground_truth_->get_ego_id());
  process(osi_sv.has_host_vehicle_data(), osi_sv.host_vehicle_data(), *osi_ego);
}

void OsiOmniSensor::process(const bool has_veh_data, const osi3::HostVehicleData& osi_hv,
                            const osi3::MovingObject& osi_ego) {
  auto obj = std::make_shared<cloe::Object>();
  obj->exist_prob = 1.0;
  // Object id
  from_osi_identifier(osi_ego.id(), obj->id);
  assert(obj->id == static_cast<int>(owner_id_));

  // Ego pose
  if (has_veh_data) {
    // Ego data that was explicitly made available to the sensor (e.g. gps
    // location & rmse).
    from_osi_host_vehicle_data(osi_hv, *obj);
  } else {
    // Use ground truth object information
    from_osi_base_moving(osi_ego.base(), *obj);
  }

  // Data extracted from ground truth:
  //  - Vehicle type
  from_osi_mov_obj_type_classification(osi_ego, obj->classification);
  //  - Offset to vehicle frame origin
  obj->cog_offset = ground_truth_->get_veh_coord_sys_info(obj->id);

  // Store ego pose.
  osi_ego_pose_ = obj->pose;
  osi_ego_pose_.translation() = obj->pose.translation() + obj->pose.rotation() * obj->cog_offset;

  // Object attributes are all set:
  //  - 1a) osi3::HostVehicleData: "All coordinates and orientations are relative
  //        to the global ground truth coordinate system."
  //  - 1b) "All position coordinates refer to the center of the bounding box of
  //         the object (vehicle or otherwise)."
  //  - 2 ) osi3::MovingObject::VehicleAttributes::bbcenter_to_rear: "The vector
  //        pointing from the bounding box center point to the middle of the rear
  //        axle under neutral load conditions. In object coordinates."
  // Now transform the data into the Cloe reference frame convention:
  //  - 1a) obj->velocity/acceleration: Convert from world frame into vehicle
  //        frame coordinates.
  //  - 1b) obj->pose: Change object position from bbox-center to vehicle
  //        reference point (rear axle/street level).
  //  - 2 ) obj->cog_offset: cog should be on street level, i.e. only x-offset is
  //        non-zero. Here, the direction is opposite as defined by OSI.
  transform_ego_coord_from_osi_data(ground_truth_->get_mov_obj_dimensions(obj->id), *obj);
  store_ego_object(obj);  // XXX is this fine for multiple sensor views?
}

void OsiOmniSensor::process(const bool has_eh, const osi3::DetectedEntityHeader& osi_eh,
                            const osi3::DetectedMovingObject& osi_mo) {
  auto obj = std::make_shared<cloe::Object>();

  // Get object information. The sensor (model) may not provide all required data.
  if (has_eh) {
    // TODO(tobias): handle entity header, if needed
    osi_logger()->warn(
        "VtdOsiSensor: DetectedEntityHeader not yet handled. measurement_time = {}ns",
        osi_timestamp_to_simtime(osi_eh.measurement_time()).count());
  }
  switch (this->get_mock_level(SensorMockTarget::DetectedMovingObject)) {
    case SensorMockLevel::OverwriteNone: {
      from_osi_detected_moving_object(osi_mo, *obj);
      break;
    }
    case SensorMockLevel::InterpolateMissing: {
      from_osi_detected_moving_object_alt(osi_mo, *ground_truth_, *obj);
      break;
    }
    case SensorMockLevel::OverwriteAll: {
      throw cloe::ModelError(
          "OSI SensorMockLevel::OverwriteAll not available for DetectedMovingObject");
      break;
    }
  }

  assert(obj->id != static_cast<int>(owner_id_));

  // Offset to the vehicle frame origin
  obj->cog_offset = ground_truth_->get_veh_coord_sys_info(obj->id);

  // Object attributes are all set:
  //  - 1a) DetectedMovingObject::base: "The parent frame of base is the sensor's
  //        [vehicle frame]."
  //  - 1b) "All position coordinates refer to the center of the bounding box of
  //         the object (vehicle or otherwise)."
  //  - 2 ) osi3::MovingObject::VehicleAttributes::bbcenter_to_rear: "The vector
  //        pointing from the bounding box center point to the middle of the rear
  //        axle under neutral load conditions. In object coordinates."
  // Now transform the data to the Cloe reference frame:
  //  - 1a) obj->pose/velocity/acceleration/angular_velocity: Transform detected
  //        object location from the ego vehicle frame into the sensor frame.
  //  - 1b) obj->pose: Change object position from bbox-center to vehicle
  //        reference point (rear axle/street level).
  //  - 2 ) obj->cog_offset: cog should be on street level, i.e. only x-offset is
  //        non-zero. Here, the direction is opposite as defined by OSI.
  transform_obj_coord_from_osi_data(osi_sensor_pose_,
                                    ground_truth_->get_mov_obj_dimensions(obj->id), *obj);

  // Fill the object list
  store_object(obj);
}

void OsiOmniSensor::from_osi_boundary_points(const osi3::LaneBoundary& osi_lb,
                                             cloe::LaneBoundary& lb) {
  assert(osi_lb.boundary_line_size() > 0);
  for (int i = 0; i < osi_lb.boundary_line_size(); ++i) {
    const auto& osi_pt = osi_lb.boundary_line(i);
    Eigen::Vector3d position = osi_vector3d_xyz_to_vector3d(osi_pt.position());
    // Transform points from the inertial into the sensor reference frame.
    cloe::utility::transform_point_to_child_frame(osi_ego_pose_, &position);
    cloe::utility::transform_point_to_child_frame(osi_sensor_pose_, &position);
    lb.points.push_back(position);
  }
  // Compute clothoid segment. TODO(tobias): implement curved segments.
  lb.dx_start = lb.points.front()(0);
  lb.dy_start = lb.points.front()(1);
  lb.heading_start = std::atan((lb.points.back()(1) - lb.points.front()(1)) /
                               (lb.points.back()(0) - lb.points.front()(0)));
  lb.curv_hor_start = 0.0;
  lb.curv_hor_change = 0.0;
  lb.dx_end = lb.points.back()(0);
}

void OsiOmniSensor::mock_detected_lane_boundaries() {
  const auto& osi_gt = ground_truth_->get_gt();
  int lb_id = 0;
  // If some of the OSI data does not have an id, avoid id clashes.
  for (const auto& osi_lb : osi_gt.lane_boundary()) {
    if (osi_lb.has_classification() && osi_lb.has_id()) {
      int id;
      from_osi_identifier(osi_lb.id(), id);
      lb_id = std::max(lb_id, id + 1);
    }
  }
  // Set lane boundary data.
  for (const auto& osi_lb : osi_gt.lane_boundary()) {
    if (osi_lb.has_classification()) {
      cloe::LaneBoundary lb;
      if (osi_lb.has_id()) {
        from_osi_identifier(osi_lb.id(), lb.id);
      } else {
        lb.id = lb_id;
      }
      lb.exist_prob = 1.0;
      lb.prev_id = -1;  // no concatenated line segments for now
      lb.next_id = -1;
      ++lb_id;
      from_osi_boundary_points(osi_lb, lb);
      lb.type = osi_lane_bdry_type_map.at(osi_lb.classification().type());
      lb.color = osi_lane_bdry_color_map.at(osi_lb.classification().color());
      store_lane_boundary(lb);
    }
  }
}

}  // namespace osii
