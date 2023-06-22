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
 * \file cloe/component/object_sensor.hpp
 * \see  cloe/component/object.hpp
 */

#pragma once

#include <cloe/component.hpp>          // for Component
#include <cloe/component/frustum.hpp>  // for Frustum
#include <cloe/component/object.hpp>   // for Object, Isometry3d
#include <fable/json.hpp>              // for Json
#include <fable/utility/eigen.hpp>

namespace cloe {

class ObjectSensor : public Component {
 public:
  using Component::Component;
  ObjectSensor() : Component("object_sensor") {}
  virtual ~ObjectSensor() noexcept = default;

  /**
   * Return the sensed world objects, whether dynamic or static.
   * "World" in this case means the environment. These may be fused or from an individual sensor.
   *
   * - The returned pointer is invalid after ClearCache is called.
   * - The ObjectSensor that returns the pointer manages the memory.
   * - The returned objects have the origin of center-rear axle.
   */
  virtual const Objects& sensed_objects() const = 0;

  /**
   * Return the frustum of the object sensor.
   */
  virtual const Frustum& frustum() const = 0;

  /**
   * Return the mounting position of the object sensor.
   */
  virtual const Eigen::Isometry3d& mount_pose() const = 0;

  /**
   * Writes JSON representation into j.
   */
  fable::Json active_state() const override {
    return fable::Json{
        {"mount_pose", this->mount_pose()},
        {"frustum", this->frustum()},
        {"sensed_objects", this->sensed_objects()},
    };
  }
};

class NopObjectSensor : public ObjectSensor {
 public:
  using ObjectSensor::ObjectSensor;
  NopObjectSensor() : ObjectSensor("nop_object_sensor") {}
  virtual ~NopObjectSensor() noexcept = default;

  const Objects& sensed_objects() const override { return objects_; }

  const Frustum& frustum() const override { return frustum_; }

  const Eigen::Isometry3d& mount_pose() const override { return mount_; }

  void reset() override {
    ObjectSensor::reset();
    objects_.clear();
  }

 protected:
  Frustum frustum_;
  Objects objects_;
  Eigen::Isometry3d mount_ = Eigen::Isometry3d::Identity();
};

}  // namespace cloe
