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
 * \file frustum_culling_test.cpp
 * \see  frustum_culling_conf.hpp
 * \see  frustum_culling_objects.cpp
 * \see  frustum_culling_lanes.cpp
 */

#include <gtest/gtest.h>

#include <fable/utility/gtest.hpp>   // for assert_validate
#include "frustum_culling_conf.hpp"  // for NoiseConf

using namespace cloe;  // NOLINT(build/namespaces)

TEST(frustum_culling, deserialize) {
  component::FrustumCullingConf c;

  fable::assert_validate(c, R"({
    "reference_frame": {
        "x": 2.5,
        "y": 0.0,
        "z": 0.6,
        "roll": 0.0,
        "pitch": 0.1,
        "yaw": 0.0
    },
    "frustum" : {
      "clip_near": 0.0,
      "clip_far": 100.0,
      "fov_h": 0.7854,
      "fov_v": 0.7854,
      "offset_h": 0.0,
      "offset_v": 0.0
    }
  })");
}
