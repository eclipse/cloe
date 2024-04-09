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
 * \file fable/schema/string_test.cpp
 * \see  fable/schema/string.hpp
 */

#include <gtest/gtest.h>

#include <fable/confable.hpp>       // for Confable
#include <fable/environment.hpp>    // for Environment
#include <fable/schema/string.hpp>  // for String, ...
#include <fable/utility/gtest.hpp>  // for assert_to_json, ...

#define TO_CONF(x) fable::Conf{fable::Json(x)}

TEST(fable_schema_string, plain) {
  std::string t;
  auto s = fable::schema::make_schema(&t, "plain");

  fable::assert_to_json(s, TO_CONF(""));
  fable::assert_from_eq_to(s, TO_CONF("hello string"));
  fable::assert_schema_eq(s, R"({
    "type": "string",
    "description": "plain"
  })");
}

TEST(fable_schema_string, not_empty) {
  std::string t;
  auto s = fable::schema::make_schema(&t, "not empty").not_empty();

  fable::assert_invalidate(s, TO_CONF(""));
  fable::assert_from_eq_to(s, TO_CONF("not empty"));
  fable::assert_schema_eq(s, R"({
    "type": "string",
    "description": "not empty",
    "minLength": 1
  })");
}

TEST(fable_schema_string, min_max_length) {
  std::string t;
  auto s = fable::schema::make_schema(&t, "min 4, max 8").min_length(4).max_length(8);

  for (const auto& x : {"", "a", "asdfasdfx"}) {
    fable::assert_invalidate(s, TO_CONF(x));
  }
  for (const auto& x : {"asdf", "xxxxxx", "abcdefgh"}) {
    fable::assert_from_eq_to(s, TO_CONF(x));
  }
  fable::assert_schema_eq(s, R"({
    "type": "string",
    "description": "min 4, max 8",
    "minLength": 4,
    "maxLength": 8
  })");
}

TEST(fable_schema_string, pattern) {
  std::string t;
  auto s = fable::schema::make_schema(&t, "c_identifier").c_identifier();

  for (const auto& x : {"0_", "0", "not identifier", "", "a-b"}) {
    fable::assert_invalidate(s, TO_CONF(x));
  }
  for (const auto& x : {"asdf", "_8", "_", "c_identier", "somethingElse"}) {
    fable::assert_from_eq_to(s, TO_CONF(x));
  }
  fable::assert_schema_eq(s, R"({
    "type": "string",
    "description": "c_identifier",
    "pattern": "^[a-zA-Z_][a-zA-Z0-9_]*$"
  })");
}

TEST(fable_schema_string, interpolate) {
  std::string t;
  fable::Environment env{
    {"TEST", "true"},
    {"NAME", "world"}
  };
  env.prefer_external(false);
  auto s = fable::schema::make_schema(&t, "interpolate").interpolate(true).environment(&env);

  EXPECT_ANY_THROW(assert_from_conf(s, TO_CONF("${}")));
  EXPECT_ANY_THROW(assert_from_conf(s, TO_CONF("${__UNLIKELY}")));

  fable::assert_from_conf(s, TO_CONF("${__UNLIKELY-default}"));
  fable::assert_to_json(s, fable::Json("default"));
  ASSERT_EQ(t, "default");

  fable::assert_from_conf(s, TO_CONF("${TEST-false}"));
  fable::assert_to_json(s, fable::Json("true"));
  ASSERT_EQ(t, "true");

  fable::assert_from_conf(s, TO_CONF("hello ${NAME}"));
  fable::assert_to_json(s, fable::Json("hello world"));
  ASSERT_EQ(t, "hello world");

  fable::assert_schema_eq(s, R"({
    "type": "string",
    "description": "interpolate"
  })");
}

TEST(fable_schema_string, enum) {
  std::string t;
  auto s = fable::schema::make_schema(&t, "enum").enum_of({"true", "false"});

  for (const auto& x : {"", "asdf", "False"}) {
    fable::assert_invalidate(s, TO_CONF(x));
  }
  for (const auto& x : {"true", "false"}) {
    fable::assert_from_eq_to(s, TO_CONF(x));
  }
  fable::assert_schema_eq(s, R"({
    "type": "string",
    "description": "enum",
    "enum": [ "true", "false" ]
  })");
}
