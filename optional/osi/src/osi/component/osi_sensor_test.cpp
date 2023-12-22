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
 * \file osi/component/osi_sensor_test.cpp
 * \see  osi/component/osi_sensor.hpp
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <osi/component/osi_sensor.hpp>

TEST(cloe_osi_sensor, sensor_data) {
  cloe_osi::NopOsiSensor sensor;
  osi3::SensorData sd;
  sd.mutable_version()->set_version_major(3);
  sd.mutable_timestamp()->set_seconds(1);

  sensor.set_sensor_data(sd);

  ASSERT_TRUE(sensor.sensor_data()->has_version());
  ASSERT_TRUE(sensor.sensor_data()->has_timestamp());
  ASSERT_FALSE(sensor.sensor_data()->has_mounting_position());
}
