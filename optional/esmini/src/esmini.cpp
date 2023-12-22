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
 * \file esmini.cpp
 *
 * This file defines the ESMini simulator plugin.
 */

#include <functional>  // for function<>
#include <memory>      // for unique_ptr<>
#include <string>      // for string
#include <vector>      // for vector<>

#include <esminiLib.hpp>

#include <cloe/component/ego_sensor.hpp>                // for NopEgoSensor
#include <cloe/component/lane_sensor.hpp>               // for LaneBoundarySensor
#include <cloe/component/latlong_actuator.hpp>          // for LatLongActuator
#include <cloe/component/object_sensor.hpp>             // for NopObjectSensor
#include <cloe/component/object_sensor_functional.hpp>  // for ObjectSensorFilter
#include <cloe/core.hpp>                                // for Duration, Error, Confable
#include <cloe/handler.hpp>                             // for ToJson
#include <cloe/models.hpp>                              // for CloeComponent
#include <cloe/plugin.hpp>                              // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                           // for Registrar
#include <cloe/simulator.hpp>                           // for Simulator
#include <cloe/sync.hpp>                                // for Sync
#include <cloe/vehicle.hpp>                             // for Vehicle

#include "esmini_conf.hpp"    // for ESMiniConfiguration
#include "esmini_logger.hpp"  // for esmini_logger()
#include "esmini_vehicle.hpp"

namespace esmini {

/**
 * Implementation of a cloe::Simulator specifically for ESMini.
 */
class ESMiniSimulator : public cloe::Simulator {
 public:
  ESMiniSimulator(const std::string& name, const ESMiniConfiguration& c)
      : Simulator(name), config_(c) {}

  virtual ~ESMiniSimulator() noexcept = default;

  void connect() final {
    SE_SetLogFilePath("/tmp/esmini.log");

    // Set seed not needed currently, as no random numbers are in use.
    // SE_SetSeed();

    // Initialize ESMini.
    int disable_ctrls = 0;  // Controllers enabled according to OpenScenario file.
    // Configure viewer.
    int use_viewer = 0;
    int threads = 0;
    if (!config_.is_headless) {
      use_viewer = 1;
      threads = 1;
    }
    int record = 0;
    auto ierr = SE_Init(config_.scenario.c_str(), disable_ctrls, use_viewer, threads, record);

    if (ierr != 0) {
      throw cloe::ModelError("ESMini initialization failed!");
    }
    configure_ego_vehicles();

    Simulator::connect();
  }

  void configure_ego_vehicles() {
    // Keep track of the requested ego vehicles.
    std::vector<std::string> ego_v;
    ego_v.resize(config_.vehicles.size());
    std::transform(config_.vehicles.begin(), config_.vehicles.end(), ego_v.begin(),
                   [](auto kv) { return kv.first; });
    // Check the scenario for the requested ego vehicles and create the cloe vehicles.
    for (int i = 0; i < SE_GetNumberOfObjects(); ++i) {
      auto id = SE_GetId(i);
      auto name = SE_GetObjectName(id);
      if (name) {
        auto it = std::find(ego_v.begin(), ego_v.end(), name);
        if (it != ego_v.end()) {
          vehicles_.push_back(std::make_shared<ESMiniVehicle>(id, name, config_.vehicles.at(name)));
          ego_v.erase(it);
        }
      }
    }
    if (ego_v.size() > 0) {
      for (auto const& ego : ego_v) {
        esmini_logger()->error("Ego vehicle not found in scenario: {}", ego);
      }
      throw cloe::ModelError("Some vehicles not found in scenario!");
    }
  }

  void disconnect() final {
    assert(!is_operational());

    SE_Close();

    Simulator::disconnect();
  }

  void reset() final {
    disconnect();
    connect();
  }

  void abort() final {}

  void enroll(cloe::Registrar& r) final {
    r.register_api_handler("/state", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<ESMiniSimulator>(this));
    r.register_api_handler("/configuration",
                           cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<ESMiniConfiguration>(&config_));
  }

  size_t num_vehicles() const final {
    assert(is_connected());
    return vehicles_.size();
  }

  std::shared_ptr<cloe::Vehicle> get_vehicle(size_t i) const final {
    assert(i < num_vehicles());
    return vehicles_[i];
  }

  std::shared_ptr<cloe::Vehicle> get_vehicle(const std::string& key) const final {
    for (const auto& v : vehicles_) {
      if (v->name() == key) {
        return v;
      }
    }
    return nullptr;
  }

  cloe::Duration process(const cloe::Sync& sync) final {
    assert(is_connected());
    assert(is_operational());
    // Receive new data from all sensors.
    cloe::Duration sensor_time{0};
    {
      for (auto v : vehicles_) {
        sensor_time = v->esmini_get_environment_data(sync);
      }
    }
    // Set actuation signals.
    for (auto v : vehicles_) {
      v->esmini_step_ego_position(sync);
    }

    // Write screenshots, if requested.
    if (config_.write_images) {
      SE_SaveImagesToFile(1);
    }

    // Trigger the next step.
    double step_size_sec = std::chrono::duration<double>(sync.step_width()).count();
    esmini_logger()->trace("Trigger timestep dt = {}s", step_size_sec);
    if (SE_StepDT(step_size_sec) != 0) {
      throw cloe::ModelError("ESMini step failed!");
    }
    auto esmini_time =
        std::chrono::duration_cast<cloe::Duration>(cloe::Seconds(SE_GetSimulationTime()));

    if (abs(esmini_time.count() - sync.time().count()) > sync.step_width().count() / 4) {
      throw cloe::Error("ESMini time {} not at Cloe time {} ns.", esmini_time.count(),
                        sync.time().count());
    }

    return sync.time();
  }

  friend void to_json(cloe::Json& j, const ESMiniSimulator& b) {
    j = cloe::Json{
        {"is_connected", b.connected_},
        {"is_operational", b.operational_},
        {"running", nullptr},
        {"num_vehicles", b.num_vehicles()},
    };
  }

 private:
  /// ESMini simulator plugin configuration.
  ESMiniConfiguration config_;
  /// Stores ego vehicles filled with environment data, driver and vehicle model.
  std::vector<std::shared_ptr<ESMiniVehicle>> vehicles_{};
};

DEFINE_SIMULATOR_FACTORY(ESMiniFactory, ESMiniConfiguration, "esmini", "basic OpenScenario player")

DEFINE_SIMULATOR_FACTORY_MAKE(ESMiniFactory, ESMiniSimulator)

}  // namespace esmini

EXPORT_CLOE_PLUGIN(esmini::ESMiniFactory)
