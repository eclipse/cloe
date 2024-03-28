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
 * \file cloe/component/frustum.hpp
 */

#pragma once

#include <cmath>  // for M_PI

#include <fable/confable.hpp>  // for Confable
#include <fable/json.hpp>      // for Json
#include <fable/schema.hpp>    // for Schema, Struct, make_schema

#define M_2X_PI (2 * M_PI)

namespace cloe {

/**
 * This class describes the frustum of a sensor.
 */
struct Frustum : public fable::Confable {
  double fov_h{M_2X_PI};
  double offset_h{0.0};
  double fov_v{M_2X_PI};
  double offset_v{0.0};
  double clip_near{0.0};
  double clip_far{480.0};

  void to_json(fable::Json& j) const override {
    j = fable::Json{
        {"fov_h", fov_h},       {"offset_h", offset_h},   {"fov_v", fov_v},
        {"offset_v", offset_v}, {"clip_near", clip_near}, {"clip_far", clip_far},
    };
  }

  void from_conf(const fable::Conf& c) override {
    assert(clip_near < clip_far);
    Confable::from_conf(c);
    if (clip_near >= clip_far) {
      c.throw_error("expect frustum near < far clipping plane, got near={} >= far={}", clip_near,
                    clip_far);
    }
  }

 protected:
  fable::Schema schema_impl() override {
    // clang-format off
    using namespace fable::schema;  // NOLINT
    return Struct{
        {"fov_h",     make_schema(&fov_h, "horizontal field of view [rad]").bounds(0, M_2X_PI)},
        {"offset_h",  make_schema(&offset_h, "horizontal field-of-view offset [rad]").bounds(-M_2X_PI, M_2X_PI)},
        {"fov_v",     make_schema(&fov_v, "vertical field of view [rad]").bounds(0, M_2X_PI)},
        {"offset_v",  make_schema(&offset_v, "vertical field-of-view offset [rad]").bounds(-M_2X_PI, M_2X_PI)},
        {"clip_near", make_schema(&clip_near, "near clipping plane [m]").minimum(0)},
        {"clip_far",  make_schema(&clip_far, "far clipping plane [m]").minimum(0)},
    };
    // clang-format on
  }

  CONFABLE_FRIENDS(Frustum)
};

}  // namespace cloe
