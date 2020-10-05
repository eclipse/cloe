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
 * \file cloe/utility/actuation_level_test.cpp
 * \see  cloe/utility/actuation_level.hpp
 */

#include <gtest/gtest.h>

#include <cloe/utility/actuation_level.hpp>
using cloe::utility::ActuationLevel;

#define NONE ActuationLevel::None
#define STANDBY ActuationLevel::Standby
#define LAT ActuationLevel::Lat
#define LONG ActuationLevel::Long
#define LATLONG ActuationLevel::LatLong

TEST(utility_actuation_level, bitwise_operations) {
  ASSERT_TRUE(ActuationLevel(static_cast<ActuationLevel::Enum>(LAT | LONG)).is_valid());
}

TEST(utility_actuation_level, invalid_states) {
  ASSERT_FALSE(ActuationLevel(static_cast<ActuationLevel::Enum>(LONG | STANDBY)).is_valid());
  ASSERT_FALSE(ActuationLevel(static_cast<ActuationLevel::Enum>(LAT | STANDBY)).is_valid());
  ASSERT_FALSE(ActuationLevel(static_cast<ActuationLevel::Enum>(LATLONG | STANDBY)).is_valid());
  ASSERT_FALSE(ActuationLevel(static_cast<ActuationLevel::Enum>(0x23)).is_valid());
}

TEST(utility_actuation_level, valid_states) {
  auto nil = ActuationLevel();
  ASSERT_TRUE(nil.is_valid());
  ASSERT_TRUE(nil.is_none());
  ASSERT_FALSE(nil.has_lat());
  ASSERT_FALSE(nil.has_long());
  ASSERT_FALSE(nil.has_control());
  ASSERT_FALSE(nil.is_standby());
  ASSERT_FALSE(nil.has_both());

  ASSERT_EQ(ActuationLevel(false, false), nil);
  ASSERT_TRUE(ActuationLevel(false, false).is_valid());
  ASSERT_TRUE(ActuationLevel(true, false).is_valid());
  ASSERT_TRUE(ActuationLevel(false, true).is_valid());
  ASSERT_TRUE(ActuationLevel(true, true).is_valid());

  auto lat = ActuationLevel(ActuationLevel::Lat);
  ASSERT_TRUE(lat.is_valid());
  ASSERT_TRUE(lat.has_lat());
  ASSERT_FALSE(lat.has_long());
  ASSERT_TRUE(lat.has_control());
  ASSERT_FALSE(lat.is_standby());
  ASSERT_FALSE(lat.has_both());

  auto lng = ActuationLevel(ActuationLevel::Long);
  ASSERT_TRUE(lng.is_valid());
  ASSERT_TRUE(lng.has_long());
  ASSERT_FALSE(lng.has_lat());
  ASSERT_TRUE(lng.has_control());
  ASSERT_FALSE(lng.is_standby());
  ASSERT_FALSE(lng.has_both());

  ASSERT_NE(lat, lng);
  ASSERT_NE(lat, nil);
  ASSERT_NE(lng, nil);

  lng.set_lat();
  ASSERT_TRUE(lng.is_valid());
  lat.set_long();
  ASSERT_TRUE(lat.is_valid());
  ASSERT_EQ(lng, lat);
  ASSERT_TRUE(lng.has_lat());
  ASSERT_TRUE(lng.has_long());
  ASSERT_TRUE(lng.has_control());
  ASSERT_FALSE(lng.is_standby());
  ASSERT_TRUE(lng.has_both());
}
