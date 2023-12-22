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
 * \file cloe/component/osi_sensor.hpp
 * \see  cloe/component/osi.hpp
 */

#pragma once
#ifndef CLOE_COMPONENT_OSI_SENSOR_HPP_
#define CLOE_COMPONENT_OSI_SENSOR_HPP_

#include <memory>  // for shared_ptr

#include "osi_groundtruth.pb.h"  // for GroundTruth
#include "osi_sensordata.pb.h"   // for SensorData
#include "osi_sensorview.pb.h"   // for SensorView

#include <cloe/component.hpp>  // for Component, Json

namespace cloe_osi {

class OsiSensor : public cloe::Component {
 public:
  using cloe::Component::Component;
  OsiSensor() : Component("osi_sensor") {}
  virtual ~OsiSensor() noexcept = default;

  /**
   * Return OSI data, if available.
   */

  virtual std::shared_ptr<osi3::GroundTruth> ground_truth() = 0;

  virtual std::shared_ptr<osi3::SensorView> sensor_view() = 0;

  virtual std::shared_ptr<osi3::SensorData> sensor_data() = 0;

  /**
   * Writes JSON representation into j.
   */
  cloe::Json active_state() const override {
    return cloe::Json{
        /*{"ground_truth", this->ground_truth()},
        {"sensor_view", this->sensor_view()},
        {"sensor_data", this->sensor_data()},*/
    };
  }
};

class NopOsiSensor : public OsiSensor {
 public:
  using OsiSensor::OsiSensor;
  NopOsiSensor() : OsiSensor("nop_osi_sensor") {}
  virtual ~NopOsiSensor() noexcept = default;

  std::shared_ptr<osi3::GroundTruth> ground_truth() override { return ground_truth_; }

  std::shared_ptr<osi3::SensorView> sensor_view() override { return sensor_view_; }

  std::shared_ptr<osi3::SensorData> sensor_data() override { return sensor_data_; }

  void set_ground_truth(const osi3::GroundTruth& gt) {
    ground_truth_ = std::make_shared<osi3::GroundTruth>(gt);
  }

  void set_sensor_view(const osi3::SensorView& view) {
    sensor_view_ = std::make_shared<osi3::SensorView>(view);
  }

  void set_sensor_data(const osi3::SensorData& data) {
    sensor_data_ = std::make_shared<osi3::SensorData>(data);
  }

  void reset() override {
    OsiSensor::reset();
    ground_truth_.reset();
    sensor_view_.reset();
    sensor_data_.reset();
  }

 protected:
  std::shared_ptr<osi3::GroundTruth> ground_truth_{nullptr};
  std::shared_ptr<osi3::SensorView> sensor_view_{nullptr};
  std::shared_ptr<osi3::SensorData> sensor_data_{nullptr};
};

}  // namespace cloe_osi

#endif  // CLOE_COMPONENT_OSI_SENSOR_HPP_
