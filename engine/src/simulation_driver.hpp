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

#pragma once

#include "simulation_sync.hpp"
#include "trigger_factory.hpp"

#include <cloe/data_broker.hpp>
#include <cloe/registrar.hpp>

namespace engine {

class Coordinator;

class SimulationDriver {
 public:
  SimulationDriver() = default;
  SimulationDriver(SimulationDriver &&) = default;
  SimulationDriver& operator=(SimulationDriver &&) = default;
  SimulationDriver(const SimulationDriver &) = delete;
  SimulationDriver& operator=(const SimulationDriver &) = delete;
  virtual ~SimulationDriver() = default;

  static cloe::Logger logger() { return cloe::logger::get("cloe"); }

  virtual void initialize(const SimulationSync &sync, Coordinator& scheduler) = 0;
  virtual void register_action_factories(cloe::Registrar& registrar) = 0;
  virtual void alias_signals(cloe::DataBroker &dataBroker) = 0;
  virtual void bind_signals(cloe::DataBroker &dataBroker) = 0;

  virtual std::vector<cloe::TriggerPtr> yield_pending_triggers() = 0;

  virtual cloe::databroker::DataBrokerBinding* data_broker_binding() { return nullptr; };

  [[nodiscard]] virtual nlohmann::json produce_report() const = 0;

  [[nodiscard]] const TriggerFactory& trigger_factory() const { return *trigger_factory_; }
  TriggerFactory& trigger_factory() { return *trigger_factory_; }

 private:
  std::unique_ptr<TriggerFactory> trigger_factory_ {std::make_unique<TriggerFactory>()};
};

}
