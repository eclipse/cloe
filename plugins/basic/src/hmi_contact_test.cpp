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
 * \file hmi_contact_test.cpp
 */

#include <chrono>             // for milliseconds
using namespace std::chrono;  // NOLINT(build/namespaces)

#include <gtest/gtest.h>  // for TEST, ASSERT_EQ, ...

#include "hmi_contact.hpp"               // for round_step<>, make_round_step_nonnegative<>
using namespace cloe::utility;           // NOLINT(build/namespaces)
using namespace cloe::utility::contact;  // NOLINT(build/namespaces)

TEST(basic_contact_round_step, positive_int) {
  ASSERT_EQ(round_step<int>(98, 10), 100);
  ASSERT_EQ(round_step<int>(10, 10), 20);
  ASSERT_EQ(round_step<int>(10, 5), 15);
}

TEST(basic_contact_round_step, negative_int) {
  ASSERT_EQ(round_step<int>(98, -10), 90);
  ASSERT_EQ(round_step<int>(100, -10), 90);
}

TEST(basic_contact_round_step, positive_double) {
  ASSERT_DOUBLE_EQ(round_step<double>(98.0, 10.0), 100.0);
  ASSERT_DOUBLE_EQ(round_step<double>(10.0, 10.0), 20.0);
}

TEST(basic_contact_round_step, negative_double) {
  ASSERT_DOUBLE_EQ(round_step<double>(98.0, -10.0), 90.0);
  ASSERT_DOUBLE_EQ(round_step<double>(100.0, -10.0), 90.0);
}

TEST(basic_contact_push_button, single) {
  double target = 36.0;

  // Test increment
  auto up = make_round_step_nonnegative<milliseconds>(&target, 10.0);
  up.update(milliseconds(3000), true);
  up.update(milliseconds(3700), true);
  ASSERT_DOUBLE_EQ(target, 36.0);
  up.update(milliseconds(3701), false);
  ASSERT_DOUBLE_EQ(target, 40.0);

  // Test decrement
  auto down = make_round_step_nonnegative<milliseconds>(&target, -10.0, -5.0);
  down.update(milliseconds(4000), true);
  down.update(milliseconds(4501), true);
  ASSERT_DOUBLE_EQ(target, 35.0);
  down.update(milliseconds(4800), true);
  ASSERT_DOUBLE_EQ(target, 30.0);
  down.update(milliseconds(5100), true);
  ASSERT_DOUBLE_EQ(target, 25.0);
  down.update(milliseconds(5401), true);
  ASSERT_DOUBLE_EQ(target, 20.0);
  down.update(milliseconds(5402), false);
  ASSERT_DOUBLE_EQ(target, 20.0);

  // Test further decrements
  down.update(milliseconds(6050), true);
  down.update(milliseconds(6100), false);
  ASSERT_DOUBLE_EQ(target, 10.0);
  down.update(milliseconds(6150), true);
  down.update(milliseconds(6200), false);
  ASSERT_DOUBLE_EQ(target, 0.0);
  down.update(milliseconds(6250), true);
  down.update(milliseconds(6300), false);
  ASSERT_DOUBLE_EQ(target, 0.0);
  down.update(milliseconds(6350), true);
  down.update(milliseconds(6400), true);
  down.update(milliseconds(6500), true);
  down.update(milliseconds(6600), true);
  down.update(milliseconds(6700), true);
  down.update(milliseconds(6800), true);
  down.update(milliseconds(6900), true);
  ASSERT_DOUBLE_EQ(target, 0.0);
}
