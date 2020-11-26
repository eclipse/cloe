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
 * \file fable/schema/const_test.cpp
 * \see  fable/schema/const.hpp
 */

#include <gtest/gtest.h>  // for TEST

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Const
#include <fable/utility/gtest.hpp>  // for assert_validate

namespace {

struct MyConstStruct : public fable::Confable {
  CONFABLE_SCHEMA(MyConstStruct) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"release", make_const_str("2", "constant string").require()},
        {"major", Const<int, Number<int>>(2, "constant number")},
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_const, schema) {
  MyConstStruct tmp;
  fable::assert_schema_eq(tmp, R"({
    "type": "object",
    "properties": {
      "release": {
        "description": "constant string",
        "const": "2"
      },
      "major": {
        "description": "constant number",
        "const": 2
      }
    },
    "required": ["release"],
    "additionalProperties": false
  })");
}

TEST(fable_schema_const, validate) {
  MyConstStruct tmp;

  fable::assert_validate(tmp, R"({
    "release": "2"
  })");

  fable::assert_validate(tmp, R"({
    "release": "2",
    "major": 2
  })");

  fable::assert_invalidate(tmp, R"({
    "release": "wrong"
  })");
}

TEST(fable_schema_const, to_json) {
  MyConstStruct tmp;
  fable::assert_to_json(tmp, R"({
    "release": "2",
    "major": 2
  })");
}
