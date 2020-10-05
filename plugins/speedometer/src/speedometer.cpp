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
 * \file speedometer.cpp
 */

#include <cloe/component.hpp>                           // for Component, ComponentFactory, ...
#include <cloe/component/utility/ego_sensor_canon.hpp>  // for EgoSensorCanon
#include <cloe/plugin.hpp>                              // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                           // for Registrar
#include <cloe/trigger/evaluate_event.hpp>              // for EvaluateCallback
#include <cloe/vehicle.hpp>                             // for Vehicle
using namespace cloe;

struct SpeedometerConf : public Confable {};

class Speedometer : public Component {
 public:
  Speedometer(const std::string& name, const SpeedometerConf&, std::shared_ptr<EgoSensor> ego)
      : Component(name, "provides an event trigger to evaluate speed in km/h"), sensor_(ego) {}

  virtual ~Speedometer() noexcept = default;

  void enroll(Registrar& r) override {
    callback_kmph_ =
        r.register_event<events::EvaluateFactory, double>("kmph", "vehicle speed in km/h");
  }

  Duration process(const Sync& sync) override {
    auto ego = utility::EgoSensorCanon(sensor_);
    callback_kmph_->trigger(sync, ego.velocity_as_kmph());
    return sync.time();
  }

  Json active_state() const override { return nullptr; }

 private:
  // State:
  std::shared_ptr<events::EvaluateCallback> callback_kmph_;
  std::shared_ptr<EgoSensor> sensor_;
};

DEFINE_COMPONENT_FACTORY(SpeedometerFactory, SpeedometerConf, "speedometer",
                         "provide an event trigger to evaluate speed in km/h")

DEFINE_COMPONENT_FACTORY_MAKE(SpeedometerFactory, Speedometer, EgoSensor)

// Register factory as plugin entrypoint
EXPORT_CLOE_PLUGIN(SpeedometerFactory)
