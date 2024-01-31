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
#include "frustum_culling_conf.hpp"          // for FrustumCullingConf

namespace cloe {

namespace component {

class ObjectFrustumCulling : public ObjectSensor {
 public:
  ObjectFrustumCulling(const std::string& name, const FrustumCullingConf& conf,
                       std::shared_ptr<ObjectSensor> obs)
      : ObjectSensor(name), config_(conf), sensor_(obs) {}

  virtual ~ObjectFrustumCulling() noexcept = default;

  const Objects& sensed_objects() const override {
    if (cached_) {
      return objects_;
    }
    for (const auto& o : sensor_->sensed_objects()) {
      auto obj = apply_frustum_culling(o);
      if (obj) {
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
    // TODO(tobias): transform coordinate system and check if inside frustum
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

}  // namespace component
}  // namespace cloe

EXPORT_CLOE_PLUGIN(cloe::component::ObjectFrustumCullingFactory)
