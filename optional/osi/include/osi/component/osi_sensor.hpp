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
 * \file cloe/component/osi_sensor.hpp
 * \see  cloe/component/osi.hpp
 */

#pragma once

#include <osi_sensordata.pb.h>  // for SensorData

namespace cloe_osi {

class OsiSensor : public cloe::Component {
 public:
  using cloe::Component::Component;
  OsiSensor() : Component("osi_sensor") {}
  virtual ~OsiSensor() noexcept = default;

  /**
   * Return OSI-SensorData
   */
  virtual const std::shared_ptr<osi3::SensorData>& get() const = 0;
  virtual std::shared_ptr<osi3::SensorData>& get() = 0;
};

}  // namespace cloe_osi
