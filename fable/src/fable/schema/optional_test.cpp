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
 * \file fable/schema/optional_test.cpp
 * \see  fable/schema/optional.hpp
 */

#include <iostream>   // for cout, endl
#include <string>     // for string
using namespace std;  // NOLINT(build/namespaces)

#include <gtest/gtest.h>
#include <boost/optional.hpp>

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Optional
#include <fable/utility/gtest.hpp>  // for assert_schema_eq, ...

struct MyOptionalStruct : public fable::Confable {
  boost::optional<std::string> str;

  CONFABLE_SCHEMA(MyOptionalStruct) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"str", make_schema(&str, "optional string")},
    };
  }
};

TEST(fable_schema_optional, schema) {
  MyOptionalStruct tmp;

  fable::assert_schema_eq(tmp, R"({
    "type": "object",
    "properties": {
      "str": {
        "description": "optional string",
        "oneOf": [
          { "type": "null" },
          { "type": "string" }
        ]
      }
    },
    "additionalProperties": false
  })");
}

TEST(fable_schema_optional, validate) {
  MyOptionalStruct tmp;

  fable::assert_validate(tmp, R"({
    "str": null
  })");
  ASSERT_FALSE(tmp.str);

  fable::assert_validate(tmp, R"({
    "str": "hello"
  })");
  ASSERT_FALSE(tmp.str);

  fable::assert_from_conf(tmp, R"({
    "str": "hello"
  })");
  ASSERT_TRUE(tmp.str);
  ASSERT_EQ(*(tmp.str), "hello");
}

TEST(fable_schema_optional, to_json) {
  MyOptionalStruct tmp1;

  fable::assert_to_json(tmp1, "{}");

  MyOptionalStruct tmp2;
  tmp2.str = "hello";
  fable::assert_to_json(tmp2, R"({
    "str": "hello"
  })");
}

TEST(fable_schema_optional, from_json) {
  MyOptionalStruct tmp;
  fable::assert_from_conf(tmp, R"({
    "str": "hello"
  })");
  ASSERT_EQ(*tmp.str, "hello");
}

TEST(fable_schema_optional, make_prototype) {
  auto a = fable::make_prototype<bool>();
  auto b = fable::make_schema(static_cast<boost::optional<bool>*>(nullptr), "");
  auto c = fable::make_prototype<boost::optional<bool>>();
}

struct MyDurationStruct : public fable::Confable {
  boost::optional<std::chrono::milliseconds> dura;
  std::map<std::string, boost::optional<bool>> map_of_bools;

  CONFABLE_SCHEMA(MyDurationStruct) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"dura", make_schema(&dura, "optional duration")},
        {"map_of_bools", make_schema(&map_of_bools, "optional map of bools")},
    };
  }
};
