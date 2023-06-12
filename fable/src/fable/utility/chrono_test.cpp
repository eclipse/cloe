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

#include <chrono>

#include <gtest/gtest.h>
#include <fable/utility/chrono.hpp>

TEST(fable_utility_chrono, to_string) {
  using namespace std::chrono_literals;
  std::vector<std::pair<std::chrono::nanoseconds, std::string>> tests = {
    {0ns, "0ns"},
    {1ns, "1ns"},
    {1ns, "1ns"},
    {5ns, "5ns"},
    {250us, "250us"},
    {45ms, "45ms"},
    {500ms, "500ms"},
    {200ms, "200ms"},
    {1s, "1s"},
    {1050ms, "1.05s"},
    {1500ms, "1.5s"},
    {30s, "30s"},
  };
  for (auto& test : tests) {
    EXPECT_EQ(test.second, fable::to_string(test.first));
  }
}

TEST(fable_utility_chrono, parse_duration) {
  using namespace std::chrono_literals;
  std::vector<std::pair<std::string, std::chrono::nanoseconds>> valid = {
    {"0ns", 0ns},
    {"1ns", 1ns},
    {"1.0ns", 1ns},
    {"5ns", 5ns},
    {"45ms", 45ms},
    {"1s", 1s},
    {"250us", 250us},
    {"250 us", 250us},
    {"0.5s", 500ms},
    {"1.5 s", 1500ms},
    {"0.2 s", 200ms},
    {"0.5min", 30s},
    {"0.5h", 1800s},
    {"0.5 s", 500ms},
    {"1.05 s", 1050ms},
    {"0.5 second", 500ms},
    {"0.5 seconds", 500ms},
    {"500 milliseconds", 500ms},
  };
  for (auto& test : valid) {
    auto result = fable::parse_duration<std::chrono::nanoseconds>(test.first);
    EXPECT_EQ(test.second, result);

    auto expect = fable::to_string(test.second);
    auto got = fable::to_string(result);
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
    ASSERT_ANY_THROW(fable::parse_duration<std::chrono::nanoseconds>(s));
  }
}

TEST(fable_utility_chrono, from_json) {
  using namespace std::chrono_literals;
  std::chrono::nanoseconds ns;
  std::chrono::milliseconds ms;

  ns = (R"("50ns")"_json).get<decltype(ns)>();
  ASSERT_EQ(ns, 50ns);

  ns = (R"("50.5s")"_json).get<decltype(ns)>();
  ASSERT_EQ(ns, 50500ms);

  ms = (R"("50.5s")"_json).get<decltype(ms)>();
  ASSERT_EQ(ms, 50500ms);
}

TEST(fable_utility_chrono, from_eq_json) {
  using namespace std::chrono_literals;
  std::chrono::nanoseconds ns = 0ns;
  std::chrono::milliseconds ms = 0ms;

  auto j = R"("50ns")"_json;
  ns = j;
  ASSERT_EQ(ns, 50ns);
  nlohmann::json test = ns;
  ASSERT_EQ(j.dump(), test.dump());

  j = R"("50.5s")"_json;
  ms = j;
  ASSERT_EQ(ms, 50500ms);
  test = ms;
  ASSERT_EQ(j.dump(), test.dump());
}
