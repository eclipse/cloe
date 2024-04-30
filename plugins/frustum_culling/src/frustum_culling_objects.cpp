/*
 * Copyright 2024 Robert Bosch GmbH
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
 * \file frustum_culling_objects.cpp
 */

#include <Eigen/Geometry>  // for Isometry3d, Vector3d
#include <memory>          // for shared_ptr<>
#include <random>          // for random_device
#include <string>          // for string

#include <cloe/component.hpp>                // for Component, Json
#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/object.hpp>         // for Object
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor
#include <cloe/conf/action.hpp>              // for actions::ConfigureFactory
#include <cloe/plugin.hpp>                   // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                // for Registrar
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/trigger/set_action.hpp>       // for actions::SetVariableActionFactory
#include <cloe/utility/frustum_culling.hpp>  // for Point, is_point_inside_frustum
#include "frustum_culling_conf.hpp"          // for FrustumCullingConf

namespace cloe::frustum_culling_plugin {

/// @brief This class rotates objects to the coordinate system of a different sensor
///
/// @details An object given in coordinate system c1 is converted to an object in coordinate system c2 via the configured reference frame in the configuration.
///          The reference frame configuration expects the values from c1 to c2,
///          e.g. if c2 is rotated by 90 degrees in mathematic positive direction from c1, the yaw should be set to +90 degree (in radians)
///          Analoguely, if the origin of c2 is translated 5 m in positive x direction from c1, the configuration should be set to +5 m.
///          The class considers first the translation in the original coordinate system (c1) and then the rotation.
class ObjectFrustumCulling : public ObjectSensor {
 public:
  ObjectFrustumCulling(const std::string& name, const FrustumCullingConf& conf,
                       std::shared_ptr<ObjectSensor> obs)
      : ObjectSensor(name), config_(conf), sensor_(obs), cached_(false) {
    config_.ref_frame.convert();
  }

  virtual ~ObjectFrustumCulling() noexcept = default;

  const Objects& sensed_objects() const override {
    if (cached_) {
      return objects_;
    }
    for (const auto& o : sensor_->sensed_objects()) {
      auto obj = apply_frustum_culling(o);

      bool is_object_inside =
          cloe::utility::is_point_inside_frustum(config_.frustum, obj->pose.translation());

      if (is_object_inside) {
        objects_.push_back(obj);
      }
    }
    cached_ = true;
    return objects_;
  }

  const Frustum& frustum() const override { return config_.frustum; }

  const Eigen::Isometry3d& mount_pose() const override { return config_.ref_frame.pose; }

  /**
   * Process the underlying sensor and clear the cache.
   *
   * We could process and create the filtered list of objects now, but we can
   * also delay it (lazy computation) and only do it when absolutely necessary.
   * This comes at the minor cost of checking whether cached_ is true every
   * time sensed_objects() is called.
   */
  Duration process(const Sync& sync) override {
    // This currently shouldn't do anything, but this class acts as a prototype
    // for How It Should Be Done.
    Duration t = ObjectSensor::process(sync);
    if (t < sync.time()) {
      return t;
    }

    // Process the underlying sensor and clear the cache.
    t = sensor_->process(sync);
    clear_cache();
    return t;
  }

  void reset() override {
    ObjectSensor::reset();
    sensor_->reset();
    clear_cache();
  }

  void abort() override {
    ObjectSensor::abort();
    sensor_->abort();
  }

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<actions::ConfigureFactory>(
        &config_, "config", "configure object sensor culling component"));
  }

 protected:
  std::shared_ptr<Object> apply_frustum_culling(const std::shared_ptr<Object>& o) const {
    auto obj = std::make_shared<Object>(*o);

    // Assumption:
    // * cog_offset is in detected objects coordinate system
    // * dimensions is in absolute values and not provided as a vector
    // * the coordinate systems do not have any relative velocity/acceleration/angular velocity, both have same velocity/acceleration/angular velocity
    obj->pose = this->mount_pose().inverse() * obj->pose;
    obj->velocity = this->mount_pose().inverse().rotation() * obj->velocity;
    obj->acceleration = this->mount_pose().inverse().rotation() * obj->acceleration;
    obj->angular_velocity = this->mount_pose().inverse().rotation() * obj->angular_velocity;
    return obj;
  }

  void clear_cache() {
    objects_.clear();
    cached_ = false;
  }

 private:
  FrustumCullingConf config_;
  std::shared_ptr<ObjectSensor> sensor_;
  mutable bool cached_;
  mutable Objects objects_;
};

DEFINE_COMPONENT_FACTORY(ObjectFrustumCullingFactory, FrustumCullingConf, "frustum_culling_objects",
                         "transform objects to given reference frame and apply frustum culling")

DEFINE_COMPONENT_FACTORY_MAKE(ObjectFrustumCullingFactory, ObjectFrustumCulling, ObjectSensor)

}  // namespace cloe::frustum_culling_plugin

EXPORT_CLOE_PLUGIN(cloe::frustum_culling_plugin::ObjectFrustumCullingFactory)
