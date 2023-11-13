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

#include <string>
#include <map>
#include <vector>

#include <gtest/gtest.h>

#include <fable/confable.hpp>
#include <fable/schema.hpp>
#include <fable/utility/gtest.hpp>

namespace {

template <typename T>
struct Nested : public fable::Confable {
  T value{}; // NOLINT

 public:
  CONFABLE_SCHEMA(Nested<T>) {
    return fable::Schema{
      { "v", fable::Schema(&value, "nested value") },
    };
  }
};

template <typename T>
struct MapOfSomething : public fable::Confable {
  std::map<std::string, T> values; // NOLINT

 public:
  CONFABLE_SCHEMA(MapOfSomething<T>) {
    return fable::Schema{
        {"values", fable::Schema(&values, "")},
    };
  }
};

}

TEST(fable_schema_map, validate_nested_1x) {
  Nested<double> wrapper;

  fable::assert_validate(wrapper, R"({
    "v": 1.0
  })");
}

TEST(fable_schema_map, validate_nested_2x) {
  Nested<Nested<double>> wrapper;

  fable::assert_validate(wrapper, R"({
    "v": { "v": 1.0 }
  })");
}

TEST(fable_schema_map, validate_nested_3x) {
  Nested<Nested<Nested<double>>> wrapper;

  fable::assert_validate(wrapper, R"({
    "v": { "v": { "v": 1.0 } }
  })");
}

TEST(fable_schema_map, validate_nested_4x) {
  Nested<Nested<Nested<Nested<double>>>> wrapper;

  fable::assert_validate(wrapper, R"({
    "v": { "v": { "v": { "v": 1.0 } } }
  })");
}

TEST(fable_schema_map, validate_nested_8x) {
  Nested<Nested<Nested<Nested<Nested<Nested<Nested<Nested<double>>>>>>>> wrapper;

  fable::assert_validate(wrapper, R"({
    "v": { "v": { "v": { "v": { "v": { "v": { "v": { "v": 1.0 } } } } } } }
  })");
}

TEST(fable_schema_map, validate_map_of_nested_2x) {
  MapOfSomething<Nested<Nested<double>>> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": {
        "v": { "v": 1.0 }
      }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_nested_3x) {
  MapOfSomething<Nested<Nested<Nested<double>>>> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": {
        "v": { "v": { "v": 1.0 } }
      }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_nested_4x) {
  MapOfSomething<Nested<Nested<Nested<Nested<double>>>>> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": {
        "v": { "v": { "v": { "v": 1.0 } } }
      }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_double) {
  MapOfSomething<double> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": 1.0,
      "b": 2.0
    }
  })");
}
