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
 * \file osi_ground_truth.hpp
 * \see  osi_ground_truth.cpp
 */

#pragma once

#include <Eigen/Geometry>  // for Vector3d

#include <cloe/simulator.hpp>  // for ModelError

#include "osi_groundtruth.pb.h"  // for GroundTruth
#include "osi_object.pb.h"       // for MovingObject

#include "osi/utility/osi_utils.hpp"  // for osi_require, ..

namespace osii {

/**
 * OsiGroundTruth provides convenient access to auxiliary ground truth
 * information while processing an OSI message.
 */
class OsiGroundTruth {
 public:
  OsiGroundTruth() = default;

  virtual ~OsiGroundTruth() = default;

  /**
   * Store address of the GroundTruth object belonging to the OSI message
   * that is to be processed.
   */
  void set(const osi3::GroundTruth& osi_gt);

  const osi3::GroundTruth& get_gt() const {
    if (!gt_ptr_) error();
    return *gt_ptr_;
  }

  void store_veh_coord_sys_info(int obj_id, const osi3::MovingObject::VehicleAttributes& osi_va) {
    // Assume that VehicleAttributes contains valid data.
    veh_bbcenter_to_rear_[obj_id] = osi_vehicle_attrib_rear_offset_to_vector3d(osi_va);
  }

  /**
   * Get the offset between coordinate reference frames of a vehicle (rear axle
   * center) and the bounding box center, e.g. for coordinate transformations.
   */
  const Eigen::Vector3d& get_veh_coord_sys_info(int obj_id) const {
    return veh_bbcenter_to_rear_.at(obj_id);
  }

  void store_mov_obj_dimensions(int obj_id, const osi3::Dimension3d& obj_dim) {
    // Assume that Dimension3d contains valid data.
    mov_obj_dimensions_[obj_id] = osi_dimension3d_lwh_to_vector3d(obj_dim);
  }

  /**
   * Get dimensions of a moving object, e.g. for coordinate transformations.
   */
  const Eigen::Vector3d& get_mov_obj_dimensions(int obj_id) const {
    return mov_obj_dimensions_.at(obj_id);
  }

  /**
   * Discard all data, e.g after processing an OSI message.
   */
  void reset() {
    gt_ptr_ = nullptr;
    veh_bbcenter_to_rear_.erase(veh_bbcenter_to_rear_.begin(), veh_bbcenter_to_rear_.end());
    mov_obj_dimensions_.erase(mov_obj_dimensions_.begin(), mov_obj_dimensions_.end());
  }

  /**
   * Get the ground truth id of the ego vehicle.
   */
  uint64_t get_ego_id() const {
    osi_require("GroundTruth::host_vehicle_id", gt_ptr_->has_host_vehicle_id());
    return gt_ptr_->host_vehicle_id().value();
  }

  /**
   * Get ground truth information for the requested moving object.
   */
  const osi3::MovingObject* get_moving_object(const uint64_t id) const;

 private:
  void error() const { throw cloe::ModelError("OsiGroundTruth not set"); }

 protected:
  /// Pointer to ground truth object of the processed OSI message.
  const osi3::GroundTruth* gt_ptr_{nullptr};

  /// Store object coordinate system info for each object <obj_id,offset>.
  std::map<int, Eigen::Vector3d> veh_bbcenter_to_rear_;

  /// Store moving object dimensions for each object <obj_id,dimensions>.
  std::map<int, Eigen::Vector3d> mov_obj_dimensions_;
};

}  // namespace osii
