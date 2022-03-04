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
 * \file cloe/utility/geometry.hpp
 */

#pragma once
#ifndef CLOE_UTILITY_GEOMETRY_HPP_
#define CLOE_UTILITY_GEOMETRY_HPP_

#include <Eigen/Geometry>

namespace cloe {
namespace utility {

/**
 * QuaternionFromRPY calculates a quaternion from roll, pitch and yaw.
 */
inline Eigen::Quaterniond quaternion_from_rpy(double roll, double pitch, double yaw) {
  // ZYX body flxed rotations
  Eigen::Quaterniond qt = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                          Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                          Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  return qt;
}

/**
 * PoseFromRotationTranslation calculates the pose from rotation and translation.
 */
inline Eigen::Isometry3d pose_from_rotation_translation(const Eigen::Quaterniond& quaternion,
                                                        const Eigen::Vector3d& trans) {
  Eigen::Isometry3d pose;
  pose.setIdentity();
  pose.linear() = quaternion.matrix();
  pose.translation() = trans;
  return pose;
}

/**
 * Compute the roll, pitch and yaw angles from a given pose.
 *  - \param pose: Pose of which the rotation matrix shall be expressed in Euler angles.
 */
inline Eigen::Vector3d get_pose_roll_pitch_yaw(const Eigen::Isometry3d& pose) {
  return pose.rotation().matrix().eulerAngles(2, 1, 0).reverse();
}

/**
 * Change a point's frame of reference from the parent frame to the child frame.
 *  - \param child_frame: Pose of the child reference frame w.r.t. the parent frame.
 *  - \param pt_vec: Point coordinate vector w.r.t. the parent frame.
 */
inline void transform_point_to_child_frame(const Eigen::Isometry3d& child_frame,
                                           Eigen::Vector3d* pt_vec) {
  *pt_vec = child_frame.inverse() * (*pt_vec);
}

/**
 * Change a point's frame of reference from the child frame to the parent frame.
 *  - \param child_frame: Pose of the child reference frame w.r.t. the parent frame.
 *  - \param pt_vec_child: Point coordinate vector w.r.t. the child frame.
 */
inline void transform_point_to_parent_frame(const Eigen::Isometry3d& child_frame,
                                            Eigen::Vector3d* pt_vec_child) {
  *pt_vec_child = child_frame * (*pt_vec_child);
}

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_GEOMETRY_HPP_
