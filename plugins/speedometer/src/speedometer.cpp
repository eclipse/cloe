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

#include <memory>  // for shared_ptr<>

#include <fable/confable.hpp>  // for Confable, CONFABLE_FRIENDS
#include <fable/json.hpp>      // for Json

#include <cloe/component.hpp>                           // for Component, ComponentFactory, ...
#include <cloe/component/utility/ego_sensor_canon.hpp>  // for EgoSensorCanon
#include <cloe/data_broker.hpp>                         // for DataBroker
#include <cloe/plugin.hpp>                              // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                           // for Registrar
#include <cloe/trigger/evaluate_event.hpp>              // for EvaluateCallback
#include <cloe/vehicle.hpp>                             // for Vehicle

struct SpeedometerConf : public fable::Confable {
  CONFABLE_FRIENDS(SpeedometerConf)
};

class Speedometer : public cloe::Component {
 public:
  Speedometer(const std::string& name, const SpeedometerConf& /*conf*/,
              std::shared_ptr<cloe::EgoSensor> ego)
      : Component(name, "provides an event trigger to evaluate speed in km/h"), sensor_(ego) {}

  ~Speedometer() noexcept override = default;

  void enroll(cloe::Registrar& r) override {
    callback_kmph_ =
        r.register_event<cloe::events::EvaluateFactory, double>("kmph", "vehicle speed in km/h");

    auto kmph_signal = r.declare_signal<double>("kmph");
    kmph_signal->set_getter<double>(
        [this]() -> double { return cloe::utility::EgoSensorCanon(sensor_).velocity_as_kmph(); });
  }

  cloe::Duration process(const cloe::Sync& sync) override {
    auto ego = cloe::utility::EgoSensorCanon(sensor_);
    callback_kmph_->trigger(sync, ego.velocity_as_kmph());
    return sync.time();
  }

  fable::Json active_state() const override {
    return fable::Json{{"kmph", cloe::utility::EgoSensorCanon(sensor_).velocity_as_kmph()}};
  }

 private:
  // State:
  std::shared_ptr<cloe::events::EvaluateCallback> callback_kmph_;
  std::shared_ptr<cloe::EgoSensor> sensor_;
};

DEFINE_COMPONENT_FACTORY(SpeedometerFactory, SpeedometerConf, "speedometer",
                         "provide an event trigger to evaluate speed in km/h")

DEFINE_COMPONENT_FACTORY_MAKE(SpeedometerFactory, Speedometer, cloe::EgoSensor)

// Register factory as plugin entrypoint
EXPORT_CLOE_PLUGIN(SpeedometerFactory)
