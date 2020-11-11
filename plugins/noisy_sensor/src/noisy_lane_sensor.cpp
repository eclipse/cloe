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
 * \file noisy_lane_sensor.cpp
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
#include "noise_data.hpp"                    // for NoiseData, NoiseConf

namespace cloe {

enum class LaneBoundaryField {
  // Lateral distance to vehicle reference point and direction [m].
  DyStart,
  // Start of road mark in driving direction [m].
  DxStart,
  // Yaw angle relative to vehicle direction [rad].
  HeadingStart,
  // Horizontal curvature at start point of the spiral [1/m].
  CurvhorStart,
  // Change of horizontal curvature at start point of the spiral [1/m^2].
  CurvhorChange,
  // Distance to last valid measurement [m].
  DxEnd
};

// clang-format off
ENUM_SERIALIZATION(LaneBoundaryField, ({
    {LaneBoundaryField::DyStart, "dy_start"},
    {LaneBoundaryField::DxStart, "dx_start"},
    {LaneBoundaryField::HeadingStart, "heading_start"},
    {LaneBoundaryField::CurvhorStart, "curv_hor_start"},
    {LaneBoundaryField::CurvhorChange, "curv_hor_change"},
    {LaneBoundaryField::DxEnd, "dx_end"},
}))
// clang-format on

namespace component {

void add_noise_dy_start(LaneBoundary* lb, const NoiseConf* noise) {
  lb->dy_start = lb->dy_start + noise->get();
}

void add_noise_dx_start(LaneBoundary* lb, const NoiseConf* noise) {
  lb->dx_start = lb->dx_start + noise->get();
}

void add_noise_heading_start(LaneBoundary* lb, const NoiseConf* noise) {
  lb->heading_start = lb->heading_start + noise->get();
}

void add_noise_curv_hor_start(LaneBoundary* lb, const NoiseConf* noise) {
  lb->curv_hor_start = lb->curv_hor_start + noise->get();
}

void add_noise_curv_hor_change(LaneBoundary* lb, const NoiseConf* noise) {
  lb->curv_hor_change = lb->curv_hor_change + noise->get();
}

void add_noise_dx_end(LaneBoundary* lb, const NoiseConf* noise) {
  lb->dx_end = lb->dx_end + noise->get();
}

class LaneNoiseConf : public NoiseConf {
 public:
  LaneNoiseConf() = default;

  virtual ~LaneNoiseConf() noexcept = default;

  /**
   * Add noise to target parameter.
   */
  std::function<void(LaneBoundary*)> apply;

  /**
   * Set the appropriate target function.
   */
  void set_target() {
    using namespace std::placeholders;  // for _1
    switch (target_) {
      case LaneBoundaryField::DyStart:
        apply = std::bind(add_noise_dy_start, _1, this);
        break;
      case LaneBoundaryField::DxStart:
        apply = std::bind(add_noise_dx_start, _1, this);
        break;
      case LaneBoundaryField::HeadingStart:
        apply = std::bind(add_noise_heading_start, _1, this);
        break;
      case LaneBoundaryField::CurvhorStart:
        apply = std::bind(add_noise_curv_hor_start, _1, this);
        break;
      case LaneBoundaryField::CurvhorChange:
        apply = std::bind(add_noise_curv_hor_change, _1, this);
        break;
      case LaneBoundaryField::DxEnd:
        apply = std::bind(add_noise_dx_end, _1, this);
        break;
    }
  }

  CONFABLE_SCHEMA(LaneNoiseConf) {
    return Schema{
        NoiseConf::schema_impl(),
        fable::schema::PropertyList<fable::schema::Box>{
            // clang-format off
            {"target", Schema(&target_, "data field of the lane boundary the noise should be applied to")},
            // clang-format on
        },
    };
  }

  void to_json(Json& j) const override {
    NoiseConf::to_json(j);
    j = Json{
        {"target", target_},
    };
  }

 private:
  LaneBoundaryField target_{LaneBoundaryField::DyStart};
};

struct NoisyLaneSensorConf : public NoisySensorConf {
  /// List of noisy lane boundary parameters.
  std::vector<LaneNoiseConf> noisy_params;

  CONFABLE_SCHEMA(NoisyLaneSensorConf) {
    return Schema{
        NoisySensorConf::schema_impl(),
        fable::schema::PropertyList<fable::schema::Box>{
            // clang-format off
              {"noise", Schema(&noisy_params, "configure noisy parameters")},
            // clang-format on
        },
    };
  }

  void to_json(Json& j) const override {
    NoisySensorConf::to_json(j);
    j = Json{
        {"noise", noisy_params},
    };
  }
};

class NoisyLaneBoundarySensor : public LaneBoundarySensor {
 public:
  NoisyLaneBoundarySensor(const std::string& name, const NoisyLaneSensorConf& conf,
                          std::shared_ptr<LaneBoundarySensor> sensor)
      : LaneBoundarySensor(name), config_(conf), sensor_(sensor) {
    reset_random();
  }

  virtual ~NoisyLaneBoundarySensor() noexcept = default;

  const LaneBoundaries& sensed_lane_boundaries() const override {
    if (cached_) {
      return lbs_;
    }
    for (const auto& kv : sensor_->sensed_lane_boundaries()) {
      auto lb = kv.second;
      apply_noise(lb);
      auto count = kv.first;
      auto ptr = std::make_shared<LaneBoundary>(lb);
      if (ptr) {
        lbs_.insert(std::pair<int, LaneBoundary>(count, lb));
      }
    }
    cached_ = true;
    return lbs_;
  }

  const Frustum& frustum() const override { return sensor_->frustum(); }

  const Eigen::Isometry3d& mount_pose() const override { return sensor_->mount_pose(); }

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
    reset_random();
  }

  void abort() override {
    LaneBoundarySensor::abort();
    sensor_->abort();
  }

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<actions::ConfigureFactory>(
        &config_, "config", "configure noisy lane component"));
    r.register_action<actions::SetVariableActionFactory<bool>>(
        "noise_activation", "switch sensor noise on/off", "enable", &config_.enabled);
  }

 protected:
  void apply_noise(LaneBoundary& lb) const {
    if (!config_.enabled) {
      return;
    }
    for (auto& np : config_.noisy_params) {
      np.apply(&lb);
    }
  }

  void reset_random() {
    // Reset the sensor's "master" seed, if applicable.
    unsigned long seed = config_.seed;
    if (seed == 0) {
      std::random_device r;
      do {
        seed = r();
      } while (seed == 0);

      if (config_.reuse_seed) {
        config_.seed = seed;
      }
    }
    for (auto& np : config_.noisy_params) {
      np.set_target();
      np.reset(seed);
      ++seed;
    }
  }

  void clear_cache() {
    lbs_.clear();
    cached_ = false;
  }

 private:
  NoisyLaneSensorConf config_;
  std::shared_ptr<LaneBoundarySensor> sensor_;
  mutable bool cached_;
  mutable LaneBoundaries lbs_;
};

DEFINE_COMPONENT_FACTORY(NoisyLaneSensorFactory, NoisyLaneSensorConf, "noisy_lane_sensor",
                         "add gaussian noise to lane sensor output")

DEFINE_COMPONENT_FACTORY_MAKE(NoisyLaneSensorFactory, NoisyLaneBoundarySensor, LaneBoundarySensor)

}  // namespace component
}  // namespace cloe

EXPORT_CLOE_PLUGIN(cloe::component::NoisyLaneSensorFactory)