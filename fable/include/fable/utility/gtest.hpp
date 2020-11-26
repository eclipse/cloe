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
#ifndef FABLE_UTILITY_GTEST_HPP_
#define FABLE_UTILITY_GTEST_HPP_

#include <iostream>  // for cerr

#include <gtest/gtest.h>  // for ASSERT_THROW, ASSERT_EQ

#include <fable/conf.hpp>      // for Conf, Json
#include <fable/confable.hpp>  // for Confable
#include <fable/error.hpp>     // for SchemaError
#include <fable/utility.hpp>   // for pretty_print

namespace fable {

inline void assert_eq(const Json& j, const Json& k) {
  ASSERT_EQ(std::string(j.dump(2)), std::string(k.dump(2)));
}

inline void assert_schema_eq(const Confable& x, const Json& expect) {
  assert_eq(x.schema().json_schema(), expect);
}

inline void assert_schema_eq(const Confable& x, const char expect[]) {
  assert_eq(x.schema().json_schema(), Json::parse(expect));
}

inline void assert_validate(const Confable& x, const Conf& input) {
  try {
    x.schema().validate(input);
  } catch (SchemaError& e) {
    pretty_print(e, std::cerr);
    throw;
  };
}

inline void assert_validate(const Confable& x, const char json_input[]) {
  assert_validate(x, Conf{Json::parse(json_input)});
}

inline void assert_invalidate(const Confable& x, const Conf& input) {
  ASSERT_THROW(x.schema().validate(input), SchemaError);
}

inline void assert_invalidate(const Confable& x, const char json_input[]) {
  assert_invalidate(x, Conf{Json::parse(json_input)});
}

inline void assert_to_json(const Confable& x, const Json& expect) {
  assert_eq(x.to_json(), expect);
}

inline void assert_to_json(const Confable& x, const char json_expect[]) {
  assert_to_json(x, Json::parse(json_expect));
}

inline void assert_from_conf(Confable& x, const Conf& input) { x.from_conf(input); }

inline void assert_from_conf(Confable& x, const char json_input[]) {
  assert_from_conf(x, Conf{Json::parse(json_input)});
}

inline void assert_from_eq_to(Confable& x, const Json& identity) {
  assert_from_conf(x, Conf{identity});
  assert_to_json(x, identity);
}

inline void assert_from_eq_to(Confable& x, const char json_input[]) {
  assert_from_eq_to(x, Json::parse(json_input));
}

}  // namespace fable

#endif  // FABLE_UTILITY_GTEST_HPP_
