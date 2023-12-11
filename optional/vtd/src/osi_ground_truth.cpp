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

#include "osi_ground_truth.hpp"  // for OsiGroundTruth

#include <cloe/simulator.hpp>  // for ModelError

#include <osi3/osi_object.pb.h>  // for MovingObject

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

}  // namespace osii
