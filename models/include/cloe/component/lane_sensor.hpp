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
 * \file cloe/component/lane_sensor.hpp
 */

#pragma once
#ifndef CLOE_COMPONENT_LANE_SENSOR_HPP_
#define CLOE_COMPONENT_LANE_SENSOR_HPP_

#include <Eigen/Geometry>             // for Isometry3d
#include <fable/json/with_eigen.hpp>  // for to_json

#include <cloe/component.hpp>                // for Component
#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/lane_boundary.hpp>  // for LaneBoundaries

namespace cloe {

class LaneBoundarySensor : public Component {
 public:
  using Component::Component;
  LaneBoundarySensor() : Component("lane_boundary_sensor") {}
  virtual ~LaneBoundarySensor() noexcept = default;

  /**
   * Return the detected lane boundaries.
   */
  virtual const LaneBoundaries& sensed_lane_boundaries() const = 0;

  /**
   * Return the frustum of the lane sensor.
   */
  virtual const Frustum& frustum() const = 0;

  /**
   * Return the mounting position of the lane sensor.
   */
  virtual const Eigen::Isometry3d& mount_pose() const = 0;

  Json active_state() const override {
    return Json{
        {"mount_pose", this->mount_pose()},
        {"frustum", this->frustum()},
        {"sensed_lane_boundaries", sensed_lane_boundaries()},
    };
  }
};

}  // namespace cloe

#endif  // CLOE_COMPONENT_LANE_SENSOR_HPP_
