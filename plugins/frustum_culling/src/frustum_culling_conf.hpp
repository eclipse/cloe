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
 * \file frustum_culling_conf.hpp
 */

#pragma once

#include <memory>   // for shared_ptr<>
#include <random>   // for default_random_engine, normal_distribution<>
#include <string>   // for string
#include <utility>  // for move

#include <cloe/component.hpp>          // for Component, ComponentFactory, ...
#include <cloe/component/frustum.hpp>  // for Frustum
#include <cloe/core.hpp>               // for Confable, Schema
#include <cloe/entity.hpp>             // for Entity
#include <cloe/simulator.hpp>          // for ModelError
#include <cloe/utility/geometry.hpp>   // for quaternion_from_rpy
#include <fable/utility/eigen.hpp>     // for to_json

namespace cloe {
namespace component {

/**
 * This class describes the frustum of a sensor.
 */
struct MountPose : public fable::Confable {
  Eigen::Isometry3d pose;
  double x, y, z;
  double roll, pitch, yaw;

  void to_json(fable::Json& j) const override {
    j = fable::Json{
        {"pose", pose},
    };
  }

  void from_conf(const fable::Conf& c) override {
    Confable::from_conf(c);
    Eigen::Quaterniond q = cloe::utility::quaternion_from_rpy(roll, pitch, yaw);
    pose = cloe::utility::pose_from_rotation_translation(q, Eigen::Vector3d(x, y, z));
  }

 protected:
  fable::Schema schema_impl() override {
    // clang-format off
    using namespace fable::schema;  // NOLINT
    return Struct{
        {"x",     make_schema(&x, "x-position in ego reference frame [m]").require()},
        {"y",     make_schema(&y, "y-position in ego reference frame [m]").require()},
        {"z",     make_schema(&z, "z-position in ego reference frame [m]").require()},
        {"roll",  make_schema(&roll, "roll angle relative to ego reference frame [rad]").require()},
        {"pitch", make_schema(&pitch, "pitch angle relative to ego reference frame [rad]").require()},
        {"yaw",   make_schema(&yaw, "yaw angle relative to ego reference frame [rad]").require()},
    };
    // clang-format on
  }

  CONFABLE_FRIENDS(MountPose)
};

struct FrustumCullingConf : public Confable {
  /**
   * Configured sensor pose used as reference frame.
   */
  MountPose ref_frame;

  /**
   * Configured sensor frustum.
   */
  Frustum frustum;

  CONFABLE_SCHEMA(FrustumCullingConf) {
    using namespace fable::schema;
    return Struct{
        {"reference_frame", make_schema(&ref_frame, "sensor frame of reference").require()},
        {"frustum", make_schema(&frustum, "sensor frustum").require()},
    };
  };

  void to_json(Json& j) const override {
    j = Json{
        {"reference_frame", ref_frame},
        {"frustum", frustum},
    };
  }
};

}  // namespace component
}  // namespace cloe
