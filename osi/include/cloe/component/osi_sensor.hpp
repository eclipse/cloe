/*
 * Copyright 2024 Robert Bosch GmbH
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
 */

#pragma once

#include <osi3/osi_sensordata.pb.h>  // for SensorData

#include <cloe/component.hpp>          // for Component
#include "cloe/utility/osi_utils.hpp"  // for osi_to_json

namespace cloe {

class OsiSensor : public Component {
 public:
  using cloe::Component::Component;
  OsiSensor() : Component("osi_sensor") {}
  OsiSensor(const OsiSensor&) = default;
  OsiSensor(OsiSensor&&) = delete;
  OsiSensor& operator=(const OsiSensor&) = default;
  OsiSensor& operator=(OsiSensor&&) = delete;
  ~OsiSensor() noexcept override = default;

  /**
   * Return OSI-SensorData
   */
  [[nodiscard]] virtual const osi3::SensorData& get() const = 0;

  /**
   * Writes JSON representation into j.
   */
  Json active_state() const override {
    return Json{
        {"sensor_data", this->get_json_str()},
    };
  }

 private:
  std::string get_json_str() const {
    std::string json_str;
    cloe::utility::osi_to_json(this->get(), &json_str);
    return json_str;
  }
};

}  // namespace cloe
