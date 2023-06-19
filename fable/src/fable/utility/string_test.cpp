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

#include <string>

#include <gtest/gtest.h>
#include <fable/utility/string.hpp>

TEST(fable_utility_string, join_vector) {
  ASSERT_EQ(fable::join_vector({"a", "b", "c"}, ""), "abc");
  ASSERT_EQ(fable::join_vector({"a", "b", "c"}, std::string_view()), "abc");
  ASSERT_EQ(fable::join_vector({"a", "b", "c"}, "-"), "a-b-c");
  ASSERT_EQ(fable::join_vector({"a"}, "-"), "a");
  ASSERT_EQ(fable::join_vector({}, "-"), "");
}

TEST(fable_utility_string, split_string) {
  ASSERT_EQ(fable::split_string("a-b-c", "-"), (std::vector<std::string>{"a", "b", "c"}));
  // ASSERT_EQ(fable::split_string("abc", "-"), (std::vector<std::string>{"abc"}));
  // ASSERT_EQ(fable::split_string("abc", ""), (std::vector<std::string>{"a", "b", "c"}));
  // ASSERT_EQ(fable::split_string("-b-", "-"), (std::vector<std::string>{"", "b", ""}));
  // ASSERT_EQ(fable::split_string("", ""), (std::vector<std::string>{}));
}
