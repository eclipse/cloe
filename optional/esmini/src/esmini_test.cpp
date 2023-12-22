/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file esmini_test.cpp
 */

#include <gtest/gtest.h>

#include <cloe/core.hpp>            // for Json
#include <fable/utility/gtest.hpp>  // for assert_validate

#include "esmini_conf.hpp"  // for ESMiniConfiguration

using namespace cloe;  // NOLINT(build/namespaces)

TEST(esmini, deserialization) {
  esmini::ESMiniConfiguration conf;

  fable::assert_validate(conf, R"({
    "headless" : true,
    "write_images": false,
    "scenario" : "/scenarios/test.xosc",
    "vehicles": {
      "Ego1": {
        "closed_loop" : true,
        "filter_distance": 200.0
      },
      "Ego2": {
        "closed_loop" : true,
        "filter_distance": 150.0
      }
    }
  })");
}
