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
 * \file fable/environment_test.cpp
 * \see  fable/environment.hpp
 */

#include <cstdlib>  // for getenv
#include <string>   // for string
#include <optional> // for optional<>

#include <gtest/gtest.h>       // for TEST, ASSERT_EQ, ...

#include <fable/environment.hpp>  // for Environment

#define ENV_VAR_HOME "HOME"
#define ENV_VAR_NONEXISTENT "NONEXISTENT_ENTRY"

std::optional<std::string> getenv_optional(const std::string& key) {
  char* v = getenv(key.c_str());
  if (v == nullptr) {
    return std::nullopt;
  } else {
    return std::string(v);
  }
}

TEST(fable_environment, get_variable) {
  fable::Environment env;
  ASSERT_NO_THROW(env.get(ENV_VAR_HOME));
  ASSERT_EQ(env.get(ENV_VAR_HOME), getenv_optional(ENV_VAR_HOME));
  if (!getenv_optional(ENV_VAR_NONEXISTENT)) {
    ASSERT_THROW(env.require(ENV_VAR_NONEXISTENT), std::out_of_range);
    ASSERT_EQ(env.get(ENV_VAR_NONEXISTENT), std::nullopt);
  }
}

TEST(fable_environment, interpolate) {
  fable::Environment env;
  env.insert("HOME", "/home/fable");

  std::string home = *getenv_optional("HOME");
  ASSERT_EQ(env.interpolate("${HOME}"), home);
  env.prefer_external(false);
  ASSERT_EQ(env.interpolate("${HOME}"), "/home/fable");

  ASSERT_EQ(env.interpolate("${NONEXISTENT_ENTRY--aha-}"), "-aha-");
  ASSERT_EQ(env.interpolate("${NONEXISTENT_ENTRY-${HOME}}"), "/home/fable");

  env.insert("MYVAR", "HOME");
  ASSERT_EQ(env.interpolate("${${MYVAR}}"), "/home/fable");
}
