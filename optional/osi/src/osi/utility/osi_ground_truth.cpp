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

#include "osi/utility/osi_ground_truth.hpp"  // for OsiGroundTruth

#include <cloe/simulator.hpp>  // for ModelError

#include "osi_object.pb.h"  // for MovingObject

namespace osii {

const osi3::MovingObject* OsiGroundTruth::get_moving_object(const uint64_t id) const {
  for (auto osi_obj = gt_ptr_->moving_object().begin(); osi_obj != gt_ptr_->moving_object().end();
       ++osi_obj) {
    if (osi_obj->id().value() == id) {
      return &(*osi_obj);
    }
  }
  throw cloe::ModelError("OSI ground truth object not found");
}

void OsiGroundTruth::set(const osi3::GroundTruth& osi_gt) {
  this->gt_ptr_ = &osi_gt;
  for (int i_mo = 0; i_mo < osi_gt.moving_object_size(); ++i_mo) {
    osi3::MovingObject osi_mo = osi_gt.moving_object(i_mo);
    int obj_id;
    osi_identifier_to_int(osi_mo.id(), obj_id);

    // Store geometric information of different object reference frames.
    if (osi_mo.has_vehicle_attributes()) {
      this->store_veh_coord_sys_info(obj_id, osi_mo.vehicle_attributes());
    }

    // Store object bounding box dimensions for cooordinate transformations.
    osi_require("GroundTruth::MovingObject::base", osi_mo.has_base());
    if (osi_mo.has_base()) {
      osi_require("GroundTruth-BaseMoving::dimension", osi_mo.base().has_dimension());
      this->store_mov_obj_dimensions(obj_id, osi_mo.base().dimension());
    }
  }
}

}  // namespace osii
