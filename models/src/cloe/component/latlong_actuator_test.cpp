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
 * \file cloe/component/latlong_actutator_test.cpp
 * \see  cloe/component/latlong_actuator.hpp
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <cloe/component/latlong_actuator.hpp>

TEST(cloe_actuation_data, acceleration) {
  const double test_acc = 3.0;  // tested example value for acceleration

  auto test_actuator = std::make_shared<cloe::LatLongActuator>();

  test_actuator->set_acceleration(test_acc);

  boost::optional<double> result = test_actuator->acceleration();

  ASSERT_TRUE(result.is_initialized());
  ASSERT_TRUE(result.get() == test_acc);
}
