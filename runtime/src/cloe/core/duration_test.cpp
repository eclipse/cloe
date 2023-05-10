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
 * \file cloe/core/duration_test.cpp
 * \see  cloe/core/duration.hpp
 */

#include <gtest/gtest.h>

#include <cloe/core/duration.hpp>
using cloe::Duration;

TEST(duration, parse_duration) {
  std::vector<std::pair<std::string, Duration>> valid = {
    {"0ns", Duration(0)},
    {"1ns", Duration(1)},
    {"1.0ns", Duration(1)},
    {"5ns", Duration(5)},
    {"45ms", Duration(45'000'000)},
    {"1s", Duration(1'000'000'000)},
    {"250us", Duration(250'000)},
    {"250 us", Duration(250'000)},
    {"0.5s", Duration(500'000'000)},
    {"1.5 s", Duration(1'500'000'000)},
    {"0.2 s", Duration(200'000'000)},
  };
  for (auto& test : valid) {
    auto expect = cloe::to_string(test.second);
    auto got = cloe::to_string(cloe::parse_duration(test.first));
    EXPECT_EQ(expect, got) << "Input value: " << test.first;
  }

  std::vector<std::string> invalid = {
    "",
    "-",
    "0",
    ".",
    "0.5ns",
  };
  for (auto& s : invalid) {
    ASSERT_ANY_THROW(cloe::parse_duration(s));
  }
}
