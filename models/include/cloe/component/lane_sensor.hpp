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

#include <Eigen/Geometry>  // for Isometry3d

#include <cloe/component.hpp>                // for Component
#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/lane_boundary.hpp>  // for LaneBoundaries
#include <fable/utility/eigen.hpp>           // for to_json

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

  fable::Json active_state() const override {
    return fable::Json{
        {"mount_pose", this->mount_pose()},
        {"frustum", this->frustum()},
        {"sensed_lane_boundaries", sensed_lane_boundaries()},
    };
  }
};

class NopLaneSensor : public LaneBoundarySensor {
 public:
  NopLaneSensor() : LaneBoundarySensor("nop_lane_sensor") {}
  virtual ~NopLaneSensor() = default;
  const LaneBoundaries& sensed_lane_boundaries() const override { return lane_boundaries_; }
  const Frustum& frustum() const override { return frustum_; }
  const Eigen::Isometry3d& mount_pose() const override { return mount_pose_; }

 private:
  LaneBoundaries lane_boundaries_;
  Frustum frustum_;
  Eigen::Isometry3d mount_pose_;
};

}  // namespace cloe
