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
 * \file fable/utility/gtest.hpp
 */

#pragma once

#include <iostream>  // for cerr

#include <gtest/gtest.h>  // for ASSERT_THROW, ASSERT_EQ

#include <fable/fable_fwd.hpp>
#include <fable/conf.hpp>      // for Conf, Json
#include <fable/confable.hpp>  // for Confable
#include <fable/schema.hpp>    // for Schema
#include <fable/error.hpp>     // for SchemaError
#include <fable/utility.hpp>   // for pretty_print

namespace fable {

/**
 * Assert that both JSON values are identical.
 *
 * This function dumps both JSON with indentation so that failing
 * tests can nicely format the diff between strings.
 */
inline void assert_eq(const Json& j, const Json& k) {
  ASSERT_EQ(std::string(j.dump(2)), std::string(k.dump(2)));
}

/**
 * Assert that both JSON values are identical.
 *
 * The second parameter is parsed to JSON and then dumped.
 *
 * This function dumps both JSON with indentation so that failing
 * tests can nicely format the diff between strings.
 */
inline void assert_eq(const Json& j, const char expect[]) {
  assert_eq(j, parse_json(expect));
}

/**
 * Assert that both JSON values are NOT identical.
 */
inline void assert_ne(const Json& j, const Json& k) {
  ASSERT_NE(std::string(j.dump(2)), std::string(k.dump(2)));
}

/**
 * Assert that both JSON values are NOT identical.
 *
 * The second parameter is parsed to JSON and then dumped.
 */
inline void assert_ne(const Json& j, const char expect[]) {
  assert_ne(j, parse_json(expect));
}

inline void assert_schema_eq(const Schema& s, const Json& expect) {
  assert_eq(s.json_schema(), expect);
}

inline void assert_schema_eq(const Schema& s, const char expect[]) {
  assert_eq(s.json_schema(), parse_json(expect));
}

inline void assert_schema_eq(const Confable& x, const Json& expect) {
  assert_eq(x.schema().json_schema(), expect);
}

inline void assert_schema_eq(const Confable& x, const char expect[]) {
  assert_eq(x.schema().json_schema(), parse_json(expect));
}

inline void assert_validate(const Schema& s, const Conf& input) {
  try {
    s.validate(input);
  } catch (SchemaError& e) {
    pretty_print(e, std::cerr);
    throw;
  };
}

inline void assert_validate(const Schema& s, const char json_input[]) {
  assert_validate(s, Conf{parse_json(json_input)});
}

inline void assert_validate(const Confable& x, const Conf& input) {
  assert_validate(x.schema(), input);
}

inline void assert_validate(const Confable& x, const char json_input[]) {
  assert_validate(x.schema(), Conf{parse_json(json_input)});
}

inline void assert_invalidate(const Schema& s, const Conf& input) {
  ASSERT_THROW(s.validate(input), SchemaError);
}

inline void assert_invalidate(const Schema& s, const char json_input[]) {
  ASSERT_THROW(s.validate(Conf{parse_json(json_input)}), SchemaError);
}

inline void assert_invalidate(const Confable& x, const Conf& input) {
  assert_invalidate(x.schema(), input);
}

inline void assert_invalidate(const Confable& x, const char json_input[]) {
  assert_invalidate(x, Conf{parse_json(json_input)});
}

/**
 * Assert that the serialization is equal to the expected JSON.
 */
template <typename T>
inline void assert_to_json(const T& x, const Json& expect) {
  assert_eq(x.to_json(), expect);
}

/**
 * Assert that the serialization is equal to the expected JSON string.
 *
 * The string is parsed to Json, so that field order is not important.
 */
template <typename T>
inline void assert_to_json(const T& x, const char json_expect[]) {
  assert_to_json(x, parse_json(json_expect));
}

inline void assert_from_conf_throw(Confable& x, const Conf& input) {
  Json before = x.to_json();
  ASSERT_THROW(x.from_conf(input), SchemaError);
  assert_eq(x.to_json(), before);
}

inline void assert_from_conf_throw(Confable& x, const char json_input[]) {
  assert_from_conf_throw(x, Conf{parse_json(json_input)});
}

template <typename T>
inline void assert_from_conf(T& x, const Conf& input) {
  x.from_conf(input);
}

template <typename T>
inline void assert_from_conf(T& x, const char json_input[]) {
  assert_from_conf(x, Conf{parse_json(json_input)});
}

/**
 * Assert that deserializing the input works and serializes to the same input.
 *
 * This asserts that the type supports the identity function. This does not need
 * to hold for any type, but you may want it to hold for your type.
 */
template <typename T>
inline void assert_from_eq_to(T& x, const Json& identity) {
  assert_from_conf(x, Conf{identity});
  assert_to_json(x, identity);
}

/**
 * Assert that deserializing the input works and serializes to the same input.
 *
 * This asserts that the type supports the identity function. This does not need
 * to hold for any type, but you may want it to hold for your type.
 */
template <typename T>
inline void assert_from_eq_to(T& x, const char json_input[]) {
  assert_from_eq_to(x, parse_json(json_input));
}

}  // namespace fable
