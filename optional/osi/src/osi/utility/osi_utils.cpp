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
 * \file osi_utils.cpp
 */

#include "osi/utility/osi_utils.hpp"

#include <fstream>  // for ofstream
#include <string>

#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <google/protobuf/util/json_util.h>  // for JsonPrint

#include <cloe/utility/geometry.hpp>  // for quaternion_from_rpy

#include "osi_common.pb.h"      // for Timestamp, Identifier, BaseMoving, ..
#include "osi_sensordata.pb.h"  // for SensorData, DetectedEntityHeader

namespace cloeosi {

template <typename OSI_T>
void osi_to_json(const OSI_T& msg, std::string* json_string) {
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = true;
  options.always_print_primitive_fields = true;
  google::protobuf::util::MessageToJsonString(msg, json_string, options);
}

template <typename OSI_T>
void osi_to_file(const OSI_T& msg, const std::string& fname) {
  std::string json;
  osi_to_json(msg, &json);
  std::ofstream out(fname);
  out << json;
  out.close();
}
template void osi_to_file(const osi3::SensorData& msg, const std::string& fname);
template void osi_to_file(const osi3::SensorView& msg, const std::string& fname);
template void osi_to_file(const osi3::GroundTruth& msg, const std::string& fname);

void osi_identifier_to_int(const osi3::Identifier& osi_id, int& id) {
  id = static_cast<int>(osi_id.value());
}

Eigen::Vector3d osi_vector3d_xyz_to_vector3d(const osi3::Vector3d osi_coord) {
  return Eigen::Vector3d(osi_coord.x(), osi_coord.y(), osi_coord.z());
}

void vector3d_to_osi_vector3d_xyz(const Eigen::Vector3d& vec, osi3::Vector3d* osi_vec) {
  osi_vec->set_x(vec(0));
  osi_vec->set_y(vec(1));
  osi_vec->set_z(vec(2));
}

Eigen::Vector3d osi_dimension3d_lwh_to_vector3d(const osi3::Dimension3d osi_dim) {
  return Eigen::Vector3d(osi_dim.length(), osi_dim.width(), osi_dim.height());
}

Eigen::Vector3d osi_orientation3d_rpy_to_vector3d(const osi3::Orientation3d osi_ori) {
  return Eigen::Vector3d(osi_ori.roll(), osi_ori.pitch(), osi_ori.yaw());
}

void vector3d_to_osi_orientation_rpy(const Eigen::Vector3d& vec, osi3::Orientation3d* osi_ori) {
  osi_ori->set_roll(vec(0));
  osi_ori->set_pitch(vec(1));
  osi_ori->set_yaw(vec(2));
}

Eigen::Vector3d osi_vehicle_attrib_rear_offset_to_vector3d(
    const osi3::MovingObject::VehicleAttributes& osi_va) {
  osi_require("VehicleAttributes::bbcenter_to_rear", osi_va.has_bbcenter_to_rear());
  return osi_vector3d_xyz_to_vector3d(osi_va.bbcenter_to_rear());
}

template <typename T>  // osi3::BaseMoving, osi3::MountingPosition, ..
Eigen::Isometry3d osi_position_orientation_to_pose(const T& osi_T) {
  osi_require("BaseMoving/MountingPosition::orientation", osi_T.has_orientation());
  Eigen::Quaterniond quaternion = cloe::utility::quaternion_from_rpy(
      osi_T.orientation().roll(), osi_T.orientation().pitch(), osi_T.orientation().yaw());

  osi_require("BaseMoving/MountingPosition::position", osi_T.has_position());
  Eigen::Vector3d translation = osi_vector3d_xyz_to_vector3d(osi_T.position());
  return cloe::utility::pose_from_rotation_translation(quaternion, translation);
}

template Eigen::Isometry3d osi_position_orientation_to_pose<osi3::BaseMoving>(
    const osi3::BaseMoving& osi_T);
template Eigen::Isometry3d osi_position_orientation_to_pose<osi3::BaseStationary>(
    const osi3::BaseStationary& osi_T);
template Eigen::Isometry3d osi_position_orientation_to_pose<osi3::MountingPosition>(
    const osi3::MountingPosition& osi_T);

void pose_to_osi_position_orientation(const Eigen::Isometry3d& pose_in, osi3::BaseMoving& base) {
  auto pos_out = base.mutable_position();
  Eigen::Vector3d pos_in = pose_in.translation();
  pos_out->set_x(pos_in(0));
  pos_out->set_y(pos_in(1));
  pos_out->set_z(pos_in(2));
  auto ori_out = base.mutable_orientation();
  Eigen::Vector3d ypr_out = pose_in.rotation().matrix().eulerAngles(2, 1, 0);
  ori_out->set_roll(ypr_out(2));
  ori_out->set_pitch(ypr_out(1));
  ori_out->set_yaw(ypr_out(0));
}

void osi_transform_base_moving(const osi3::BaseMoving& base_ref, osi3::BaseMoving& base) {
  Eigen::Isometry3d pose_ref = osi_position_orientation_to_pose(base_ref);
  Eigen::Isometry3d pose = osi_position_orientation_to_pose(base);

  // Transform base.position/orientation/velocity/acceleration/orientation_rate:
  Eigen::Vector3d pos_ref_frame = pose.translation();
  cloe::utility::transform_point_to_child_frame(pose_ref, &pos_ref_frame);
  pose.translation() = pos_ref_frame;          // location in reference frame
  pose.rotate(pose_ref.rotation().inverse());  // orientation in reference frame
  // Write new pose to base.
  pose_to_osi_position_orientation(pose, base);

  // velocity
  osi_require("BaseMoving::velocity", base.has_velocity());
  Eigen::Vector3d vec =
      pose_ref.rotation().inverse() * (osi_vector3d_xyz_to_vector3d(base.velocity()) -
                                       osi_vector3d_xyz_to_vector3d(base_ref.velocity()));
  vector3d_to_osi_vector3d_xyz(vec, base.mutable_velocity());

  // acceleration
  osi_require("BaseMoving::acceleration", base.has_acceleration());
  vec = pose_ref.rotation().inverse() * (osi_vector3d_xyz_to_vector3d(base.acceleration()) -
                                         osi_vector3d_xyz_to_vector3d(base_ref.acceleration()));
  vector3d_to_osi_vector3d_xyz(vec, base.mutable_acceleration());

  // angular velocity
  osi_require("BaseMoving::orientation_rate", base.has_orientation_rate());
  vec = pose_ref.rotation().inverse() *
        (osi_orientation3d_rpy_to_vector3d(base.orientation_rate()) -
         osi_orientation3d_rpy_to_vector3d(base_ref.orientation_rate()));
  vector3d_to_osi_orientation_rpy(vec, base.mutable_orientation_rate());
}

}  // namespace cloeosi
