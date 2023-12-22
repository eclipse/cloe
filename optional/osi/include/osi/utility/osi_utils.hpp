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
 * \file osi_utils.hpp
 * \see  osi_utils.cpp
 */

#pragma once

#include <string>

#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <cloe/core.hpp>       // for Logger, get
#include <cloe/simulator.hpp>  // for ModelError

#include "osi_common.pb.h"      // for Dimension3d, Vector3d, ..
#include "osi_sensordata.pb.h"  // for SensorData

#undef osi_require

#ifdef NDEBUG
#define osi_require(name, test_expr) ((void)0)
#else
#define osi_require(name, test_expr) \
  ((test_expr) ? (void)0 : throw cloe::ModelError("OSI message: {} required!", name))
#endif

namespace cloeosi {

inline cloe::Logger osi_logger() { return cloe::logger::get("vtd/osi"); }

/**
 * Write OSI message to a .json file.
 */
template <typename OSI_T>
void osi_to_file(const OSI_T& msg, const std::string& fname);

void osi_identifier_to_int(const osi3::Identifier& osi_id, int& id);

/**
 * Convert osi3::Vector3d (x, y, z) into Eigen::Vector3d.
 */
Eigen::Vector3d osi_vector3d_xyz_to_vector3d(const osi3::Vector3d osi_coord);

/**
 * Convert Eigen::Vector3d into osi3::Vector3d (x, y, z).
 */
void vector3d_to_osi_vector3d_xyz(const Eigen::Vector3d& vec, osi3::Vector3d* osi_vec);

/**
 * Convert a osi3::Dimension3d (l, w, h) into Eigen::Vector3d.
 */
Eigen::Vector3d osi_dimension3d_lwh_to_vector3d(const osi3::Dimension3d osi_dim);

/**
 * Convert a osi3::Orientation3d (r, p, y) into Eigen::Vector3d.
 */
Eigen::Vector3d osi_orientation3d_rpy_to_vector3d(const osi3::Orientation3d osi_ori);

/**
 * Convert vehicle attribute bbcenter_to_rear into Eigen::Vector3d.
 */
Eigen::Vector3d osi_vehicle_attrib_rear_offset_to_vector3d(
    const osi3::MovingObject::VehicleAttributes& osi_va);

/**
 * Convert OSI position and orientation to pose.
 */
template <typename T>
Eigen::Isometry3d osi_position_orientation_to_pose(const T& osi_T);

/**
 * Convert object pose to OSI BaseMoving.
 */
void pose_to_osi_position_orientation(const Eigen::Isometry3d&, osi3::BaseMoving& base);

/**
 * Transform OSI BaseMoving into given reference frame.
 *
 * \param base_ref Reference frame position/orientation in common coordinate system.
 * \param base Base in common coordinate system, to be transformed into base_ref frame.
 */
void osi_transform_base_moving(const osi3::BaseMoving& base_ref, osi3::BaseMoving& base);

}  // namespace cloeosi
