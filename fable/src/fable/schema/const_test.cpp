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

#include <iostream>   // for cout, endl
#include <string>     // for string
using namespace std;  // NOLINT(build/namespaces)

#include <gtest/gtest.h>

#include <fable/confable.hpp>  // for Confable
#include <fable/schema.hpp>    // for Const

struct MyConstStruct : public fable::Confable {
  CONFABLE_SCHEMA(MyConstStruct) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"release", make_const_str("2", "constant string").require()},
        {"major", Const<int, Number<int>>(2, "constant number")},
    };
  }
};

inline void assert_json_eq(const fable::Json& j, const fable::Json& k) {
  ASSERT_EQ(std::string(j.dump()), std::string(k.dump()));
}

TEST(fable_schema_const, schema) {
  MyConstStruct tmp;

  fable::Json expect = R"({
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
  })"_json;

  assert_json_eq(tmp.schema().json_schema(), expect);
}

TEST(fable_schema_const, validate) {
  MyConstStruct tmp;

  tmp.schema().validate(fable::Conf{R"({
    "release": "2"
  })"_json});

  tmp.schema().validate(fable::Conf{R"({
    "release": "2",
    "major": 2
  })"_json});

  fable::Conf wrong{R"({
    "release": "wrong"
  })"_json};
  ASSERT_ANY_THROW(tmp.schema().validate(wrong));
}

TEST(fable_schema_const, to_json) {
  MyConstStruct tmp;
  fable::Json expect = R"({
    "release": "2",
    "major": 2
  })"_json;
  assert_json_eq(tmp.schema().to_json(), expect);
}
