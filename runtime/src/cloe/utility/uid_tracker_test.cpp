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
 * \file cloe/utility/uid_tracker_test.cpp
 * \see  cloe/utility/uid_tracker.hpp
 */

#include <gtest/gtest.h>

#include <cloe/utility/uid_tracker.hpp>
using cloe::utility::UniqueIDTracker;

TEST(utility_unique_id_tracker_test, with_2) {
  auto tracker = UniqueIDTracker(1, 2);

  int a_in = 3, b_in = 6, c_in = 1;
  auto a = tracker.assign(a_in);
  ASSERT_TRUE(a >= 1 && a <= 2);
  auto b = tracker.assign(b_in);
  ASSERT_TRUE(b >= 1 && b <= 2);
  ASSERT_NE(a, b);

  // Make sure they are the same in the next cycle
  tracker.next_cycle();
  ASSERT_EQ(b, tracker.assign(b_in));
  ASSERT_EQ(a, tracker.assign(a_in));
  ASSERT_EQ(b, tracker.assign(b_in)) << "Multiple assignment should be ok";

  // Trying to track a new value should throw an exception
  ASSERT_ANY_THROW(tracker.assign(c_in));

  tracker.next_cycle();
  ASSERT_EQ(a, tracker.assign(a_in));
  ASSERT_ANY_THROW(tracker.assign(c_in));

  tracker.next_cycle();
  ASSERT_EQ(b, tracker.assign(c_in));
  ASSERT_EQ(a, tracker.assign(a_in));
}
