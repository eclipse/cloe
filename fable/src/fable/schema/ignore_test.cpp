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
 * \file fable/schema/ignore_test.cpp
 * \see  fable/schema/ignore.hpp
 */

#include <gtest/gtest.h>  // for TEST

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Ignore
#include <fable/utility/gtest.hpp>  // for assert_validate

namespace {

struct MyIgnoreStruct : public fable::Confable {
  CONFABLE_SCHEMA(MyIgnoreStruct) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"args", Ignore("validates with anything")},
        {"default", Ignore()},
        {"silent", Ignore("")},
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_ignore, schema) {
  MyIgnoreStruct tmp;
  fable::assert_schema_eq(tmp, R"({
    "type": "object",
    "properties": {
      "args": {
        "description": "validates with anything"
      },
      "default": {
        "description": "ignored"
      },
      "silent": {}
    },
    "additionalProperties": false
  })");
}
