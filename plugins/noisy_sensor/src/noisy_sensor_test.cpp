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
 * \file noisy_sensor_test.cpp
 * \see  noise_data.hpp
 * \see  noisy_object_sensor.cpp
 * \see  noisy_lane_sensor.cpp
 */

#include <gtest/gtest.h>

#include <fable/utility/gtest.hpp>  // for assert_validate
#include "noise_data.hpp"           // for NoiseConf

using namespace cloe;  // NOLINT(build/namespaces)

TEST(noisy_sensor, deserialize_distribution) {
  component::NoiseConf n;

  fable::assert_validate(n, R"({
    "distribution": {
        "binding": "normal",
        "args": {
            "mean": 1.0,
            "std_deviation": 0.1
        }
    }
  })");
}