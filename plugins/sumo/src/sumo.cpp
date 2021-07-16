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
 */
/**
 * \file sumo.cpp
 */
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>

#include <utils/traci/TraCIAPI.h>  // for Sumo Connection

#include <cloe/plugin.hpp>     // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>  // for Registrar
#include <cloe/simulator.hpp>  // for Simulator
#include <cloe/sync.hpp>       // for Sync

#include "sumo.hpp"
#include "sumo_vehicle.hpp"

namespace cloe {
namespace sumo {

class SumoBinding : public Simulator {
 public:
  SumoBinding(const std::string &name, const SumoConfiguration &config)
      : Simulator(name), config_(config) {}

  virtual ~SumoBinding() noexcept = default;

  bool is_connected() const final { return connected_; }

  virtual void connect() final {
    egos_ = config_.egos;
    populateSumoVehicles();

    try {
      client_.connect(config_.ip_addr, config_.port);
      connected_ = true;
    } catch (tcpip::SocketException &ex) {
      logger()->info("tcp-ip socket exception during connection to the sumo client ");
      throw ex;
    }
  }

  virtual void disconnect() final {
    try {
      client_.close();
      connected_ = false;
    } catch (libsumo::TraCIException &ex) {
      logger()->info("traci exception during close of connection");
      throw ex;
    }
  }

  virtual void enroll(Registrar &r) final {
    r.register_api_handler("/configuration", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SumoConfiguration>(&config_));
  }

  virtual void finally(const Sync &) final {
    if (is_connected()) {
      disconnect();
    }
  }

  Duration process(const Sync &sync) final {
    try {
      TraCIAPI::VehicleScope vehicle_scope(client_);
      client_.simulationStep(sync.step());
      logger()->info("Sumo Vehicle = {} current speed = {} ", vehicles_[0]->name(),
                     vehicle_scope.getSpeed(vehicles_[0]->name()));
      return sync.time();
    }

    catch (libsumo::TraCIException &ex) {
      logger()->info("traci exception during simulation ");
      throw ex;
    }
  }

  size_t num_vehicles() const final { return vehicles_.size(); }

  std::shared_ptr<Vehicle> get_vehicle(size_t i) const final { return vehicles_[i]; }
  std::shared_ptr<Vehicle> get_vehicle(const std::string &key) const final {
    for (auto vehicle : vehicles_) {
      if (vehicle->name() == key) {
        return vehicle;
      }
    }
    return std::shared_ptr<SumoVehicle>{nullptr};
  }

  virtual bool can_step() const final { return true; }

  virtual void stop() final {}

 private:
  void populateSumoVehicles() {
    std::uint64_t index = 0;
    std::vector<std::string>::iterator it = egos_.begin();
    while (it != egos_.end()) {
      std::string name = *it;
      std::shared_ptr<SumoVehicle> vec(new SumoVehicle(index, name));
      vehicles_.push_back(vec);
      it++;
      index++;
    }
  }

  std::vector<std::string> egos_;
  std::vector<std::shared_ptr<SumoVehicle>> vehicles_;

  TraCIAPI client_;
  bool connected_{false};
  SumoConfiguration config_{};
};

DEFINE_SIMULATOR_FACTORY(SumoFactory, SumoConfiguration, "sumo", "Sumo")
DEFINE_SIMULATOR_FACTORY_MAKE(SumoFactory, SumoBinding)
}  // namespace sumo
}  // namespace cloe

EXPORT_CLOE_PLUGIN(cloe::sumo::SumoFactory)
