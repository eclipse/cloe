/*
 * Copyright 2021 Robert Bosch GmbH
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
 * \file gndtruth_extractor_test.cpp
 */

#include <gtest/gtest.h>

#include <cloe/core.hpp>            // for Json
#include <fable/utility/gtest.hpp>  // for assert_validate
#include "gndtruth_extractor.hpp"

using namespace cloe::controller;  // NOLINT(build/namespaces)

TEST(gndtruth_extractor, gndtruth_extractor_schema) {
  GndTruthExtractorConfiguration tmp;
  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": false,
    "properties": {
        "components": {
        "description": "array of components to be extracted",
        "items": {
            "type": "string"
        },
        "type": "array"
        },
        "output_file": {
        "description": "file path to write groundtruth output to",
        "type": "string"
        },
        "output_type": {
        "description": "type of output file to write",
        "enum": [
          "json.bz2",
          "json.gz",
          "json.zip",
          "json",
          "msgpack.bz2",
          "msgpack.gz",
          "msgpack.zip",
          "msgpack"
        ],
        "type": "string"
        }
    },
    "type": "object"
  })");
}

TEST(gndtruth_extractor, deserialization) {
  GndTruthExtractorConfiguration tmp;
  fable::assert_validate(tmp, R"({
    "components": [
        "plugin_01",
        "plugin_02"
    ],
    "output_file": "test.json.zip",
    "output_type": "json.zip"
  })");
}
