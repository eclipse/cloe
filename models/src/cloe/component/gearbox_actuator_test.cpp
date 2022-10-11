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
 * \file cloe/component/gearbox_actutator_test.cpp
 * \see  cloe/component/gearbox_actuator.hpp
 */

#include <gtest/gtest.h>

#include <cloe/component/gearbox_actuator.hpp>

TEST(cloe_gearbox_actuation, is_set) {
  cloe::GearboxActuator gearboxActuator{};

  // expect that the optional member is not set
  EXPECT_FALSE(gearboxActuator.is_set());

  // set a gearbox request
  cloe::GearboxRequest testRequest{};
  gearboxActuator.set(testRequest);

  // expect that the optional member is set
  EXPECT_TRUE(gearboxActuator.is_set());

  // reset the actuator
  gearboxActuator.reset();

  // expect that the optional member is not set
  EXPECT_FALSE(gearboxActuator.is_set());
}

TEST(cloe_gearbox_actuation, set_values) {
  // intialization
  cloe::GearboxActuator gearboxActuator{};
  cloe::GearboxRequest testRequest{};
  int valueToTest = 3;

  // set the test value
  testRequest.gear_selector = valueToTest;
  gearboxActuator.set(testRequest);

  // expect that the actuator returns the same value as set
  EXPECT_EQ(gearboxActuator.get().gear_selector, valueToTest);
}
