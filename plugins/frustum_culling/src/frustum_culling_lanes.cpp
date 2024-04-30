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
 * \file frustum_culling_lanes.cpp
 */

#include <Eigen/Geometry>  // for Isometry3d
#include <memory>          // for shared_ptr<>
#include <random>          // for random_device
#include <string>          // for string
#include <utility>         // for pair

#include <cloe/component.hpp>                // for Component, Json
#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/lane_sensor.hpp>    // for LaneBoundarySensor
#include <cloe/conf/action.hpp>              // for actions::ConfigureFactory
#include <cloe/plugin.hpp>                   // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                // for Registrar
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/trigger/set_action.hpp>       // for actions::SetVariableActionFactory
#include "frustum_culling_conf.hpp"          // for NoiseData, NoiseConf

namespace cloe::frustum_culling_plugin {

class LaneBoundaryFrustumCulling : public LaneBoundarySensor {
 public:
  LaneBoundaryFrustumCulling(const std::string& name, const FrustumCullingConf& conf,
                             std::shared_ptr<LaneBoundarySensor> obs)
      : LaneBoundarySensor(name), config_(conf), sensor_(obs) {}

  virtual ~LaneBoundaryFrustumCulling() noexcept = default;

  const LaneBoundaries& sensed_lane_boundaries() const override {
    if (cached_) {
      return lbs_;
    }
    for (const auto& kv : sensor_->sensed_lane_boundaries()) {
      auto lb = kv.second;
      auto lb_mod = apply_frustum_culling(lb);
      lbs_.insert(std::pair<int, LaneBoundary>(kv.first, lb_mod));
    }
    cached_ = true;
    return lbs_;
  }

  const Frustum& frustum() const override { return config_.frustum; }

  const Eigen::Isometry3d& mount_pose() const override { return config_.ref_frame.pose; }

  /**
   * Process the underlying sensor and clear the cache.
   *
   * We could process and create the filtered list of lane boundaries now, but we can
   * also delay it (lazy computation) and only do it when absolutely necessary.
   * This comes at the minor cost of checking whether cached_ is true every
   * time sensed_lane_boundaries() is called.
   */
  Duration process(const Sync& sync) override {
    // This currently shouldn't do anything, but this class acts as a prototype
    // for How It Should Be Done.
    Duration t = LaneBoundarySensor::process(sync);
    if (t < sync.time()) {
      return t;
    }

    // Process the underlying sensor and clear the cache.
    t = sensor_->process(sync);
    clear_cache();
    return t;
  }

  void reset() override {
    LaneBoundarySensor::reset();
    sensor_->reset();
    clear_cache();
  }

  void abort() override {
    LaneBoundarySensor::abort();
    sensor_->abort();
  }

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<actions::ConfigureFactory>(
        &config_, "config", "configure lane sensor culling component"));
  }

 protected:
  LaneBoundary apply_frustum_culling(const LaneBoundary& lb) const {
    LaneBoundary lb_m = lb;
    // TODO(tobias): transform coordinate system and check if inside frustum
    return lb_m;
  }

  void clear_cache() {
    lbs_.clear();
    cached_ = false;
  }

 private:
  FrustumCullingConf config_;
  std::shared_ptr<LaneBoundarySensor> sensor_;
  mutable bool cached_;
  mutable LaneBoundaries lbs_;
};

DEFINE_COMPONENT_FACTORY(
    LaneBoundaryFrustumCullingFactory, FrustumCullingConf, "frustum_culling_lanes",
    "transform lane boundaries to given reference frame and apply frustum culling")

DEFINE_COMPONENT_FACTORY_MAKE(LaneBoundaryFrustumCullingFactory, LaneBoundaryFrustumCulling,
                              LaneBoundarySensor)

}  // namespace cloe::frustum_culling_plugin

EXPORT_CLOE_PLUGIN(cloe::frustum_culling_plugin::LaneBoundaryFrustumCullingFactory)
