/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file cloe/component/utility/steering_utils_test.cpp
 * \see  cloe/component/utility/steering_utils.cpp
 */

#include <gtest/gtest.h>
#include <cloe/component/utility/steering_utils.hpp>

TEST(steering_utils, test_front_left) {
  using namespace cloe::utility;
  using namespace std;

  // values of tuple: Geometry(wheel_base, track_base), center of steering angle,
  // upper limit of expected value, lower limit of expected value
  vector<tuple<Geometry, double, double, double>> test_vector = {
      make_tuple(Geometry{3.0, 1.5}, 0.0, 0.01, 0.00),
      make_tuple(Geometry{3.0, 1.5}, 0.7853, 0.95, 0.7853),   // pi/4 as input
      make_tuple(Geometry{3.0, 3.0}, 0.7853, 1.15, 0.7853),   // pi/4 as input
      make_tuple(Geometry{4.0, 1.5}, -0.7853, -0.6, -0.7853)  // -pi/4 as input
  };

  for (auto test_tuple : test_vector) {
    // call the function under test for the different input tuples for FrontLeft wheel
    double result =
        calculate_wheel_angle(get<0>(test_tuple), WheelId::FrontLeft, get<1>(test_tuple));

    EXPECT_LE(result, get<2>(test_tuple));
    EXPECT_GE(result, get<3>(test_tuple));
  }
}

TEST(steering_utils, test_front_right) {
  using namespace cloe::utility;
  using namespace std;

  // values of tuple: Geometry(wheel_base, track_base), center of steering angle,
  // upper limit of expected value, lower limit of expected value
  vector<tuple<Geometry, double, double, double>> test_vector = {
      make_tuple(Geometry{3.0, 1.5}, 0.0, 0.01, 0.00),
      make_tuple(Geometry{3.0, 1.5}, 0.7853, 0.7853, 0.6),    // pi/4 as input
      make_tuple(Geometry{3.0, 3.0}, 0.7853, 0.7853, 0.55),   // pi/4 as input
      make_tuple(Geometry{4.0, 1.5}, -0.7853, -0.7853, -0.9)  // -pi/4 as input
  };

  for (auto test_tuple : test_vector) {
    // call the function under test for the different input tuples for FrontRight wheel
    double result =
        calculate_wheel_angle(get<0>(test_tuple), WheelId::FrontRight, get<1>(test_tuple));

    EXPECT_LE(result, get<2>(test_tuple));
    EXPECT_GE(result, get<3>(test_tuple));
  }
}

TEST(steering_utils, death_test_assertion) {
  using namespace cloe::utility;
  using namespace std;

  // values of tuple: Geometry, center steering angle
  vector<tuple<Geometry, double>> test_vector = {
      make_tuple(Geometry{-1.0, 1.5}, 0.0),     // negative wheel_base as input
      make_tuple(Geometry{3.0, -1.0}, 0.7853),  // negative track_base as input
      make_tuple(Geometry{3.0, 1.5}, 0.5)       // correct input in terms of geometry
  };

#ifndef NDEBUG
  // expect that the assert is triggered for test_vector[0] as wheel_base is negative.
  EXPECT_DEATH(
      calculate_wheel_angle(get<0>(test_vector[0]), WheelId::FrontRight, get<1>(test_vector[0])),
      "");

  // expect that the assert is triggered for test_vector[1] as track_base is negative.
  EXPECT_DEATH(
      calculate_wheel_angle(get<0>(test_vector[1]), WheelId::FrontRight, get<1>(test_vector[1])),
      "");

  // expect that the assert is triggered for WheelId::RearLeft as input
  EXPECT_DEATH(
      calculate_wheel_angle(get<0>(test_vector[2]), WheelId::RearLeft, get<1>(test_vector[2])), "");

  // expect that the assert is triggered for WheelId::RearRight as input
  EXPECT_DEATH(
      calculate_wheel_angle(get<0>(test_vector[2]), WheelId::RearRight, get<1>(test_vector[2])),
      "");

#endif
}
