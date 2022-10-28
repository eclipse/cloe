/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file fable/schema/enum_test.cpp
 * \see  fable/schema/enum.hpp
 */

#include <gtest/gtest.h>

#include <fable/json.hpp> // for Json
#include <fable/schema.hpp>
#include <fable/schema/array.hpp> // for Array
#include <fable/utility/gtest.hpp>  // for assert_to_json, ...

using Vec3d = std::array<double,3>;

TEST(fable_schema_array, vec3d_schema) {
  Vec3d target;
  auto sma = fable::schema::make_schema(&target, "").require_all(true);
  fable::assert_eq(sma.json_schema(), R"({
    "type": "array",
    "items": {
      "maximum": 1.7976931348623157e+308,
      "minimum": -1.7976931348623157e+308,
      "type": "number"
    },
    "maxItems": 3,
    "minItems": 3
  })");
}

TEST(fable_schema_array, vec3d_from_json) {
  Vec3d target;
  auto sma = fable::schema::make_schema(&target, "").require_all(true);
  auto valid_inputs = std::vector<std::tuple<fable::Json, Vec3d>>{
    {"[0.0, 1.0, 2.0]"_json, Vec3d{0.0, 1.0, 2.0}},
    {"[0, 1, 2]"_json, Vec3d{0.0, 1.0, 2.0}},
  };
  auto invalid_inputs = std::vector<fable::Json>{
    R"({ "0": 0, "1": 1, "2": 2 })"_json,   // wrong type object
    R"(null)"_json, // wrong type null
    R"([])"_json, // empty array
    R"([1, 2])"_json, // array with less items
    R"([1, 2, 3, 4])"_json, // array with more items
    R"([1, 2, null])"_json, // array with wrong item
    R"([1, 2, "3"])"_json, // array with wrong item
  };

  for (auto& input : valid_inputs) {
    sma.from_conf(fable::Conf(std::get<0>(input)));
    ASSERT_EQ(target, std::get<1>(input));
  }

  for (auto& input : invalid_inputs) {
    ASSERT_THROW(sma.validate(input), fable::SchemaError);
  }
}

using MyBitset = std::array<bool, 4>;

TEST(fable_schema_array, bitset_schema) {
  MyBitset target;
  auto sma = fable::schema::make_schema(&target, "");
  fable::assert_eq(sma.json_schema(), R"({
    "oneOf": [
      {
        "items": {
          "type": "boolean"
        },
        "maxItems": 4,
        "minItems": 4,
        "type": "array"
      },
      {
        "additionalProperties": false,
        "patternProperties": {
          "^[0-9]+$": {
            "type": {
              "type": "boolean"
            }
          }
        },
        "type": "object"
      }
    ]
  })");
}

TEST(fable_schema_array, bitset_from_conf) {
  MyBitset target;
  target.fill(true);
  auto sma = fable::schema::make_schema(&target, "");
  auto valid_inputs = std::vector<std::tuple<fable::Json, MyBitset>>{
    {"[false, false, false, false]"_json, MyBitset{false, false, false, false}},
    {R"({"0": true, "2": true})"_json, MyBitset{true, false, true, false}},
    {R"({"0": false})"_json, MyBitset{false, false, true, false}},
  };

  for (auto& input : valid_inputs) {
    sma.from_conf(fable::Conf(std::get<0>(input)));
    ASSERT_EQ(target, std::get<1>(input));
  }

  auto invalid_inputs = std::vector<fable::Json>{
    R"({ "0": 0, "1": 1, "2": 2 })"_json,   // wrong type object
    R"(null)"_json, // wrong type null
    R"([])"_json, // empty array
    R"([true, false])"_json, // array with less items
    R"([true, true, true, true, true])"_json, // array with wrong type
    R"([1, 2, null])"_json, // array with wrong item
    R"([1, 2, "3"])"_json, // array with wrong item
    R"({ "-1": false })"_json, // array out-of-index
    R"({ "4": false })"_json, // array out-of-index
    R"({ "a": false })"_json, // not an integer
    R"({ "2a": false })"_json, // not an integer
    R"({ "02": false })"_json, // ambiguous number
  };

  for (auto& input : invalid_inputs) {
    ASSERT_THROW(sma.validate(input), fable::SchemaError);
  }
}

TEST(fable_schema_array, bitset_to_json) {
  MyBitset target;
  target.fill(true);
  target[0] = false;
  target[2] = false;

  auto sma = fable::Schema(&target, "");
  fable::assert_eq(sma.to_json(), R"([false, true, false, true])");
}
