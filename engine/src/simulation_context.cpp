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

#include "simulation_context.hpp"

#include <cloe/vehicle.hpp>
#include <cloe/controller.hpp>
#include <cloe/simulator.hpp>

namespace engine {

std::string SimulationContext::version() const { return CLOE_ENGINE_VERSION; }

std::vector<std::string> SimulationContext::model_ids() const {
  std::vector<std::string> v;
  v.reserve(simulators.size() + controllers.size() + vehicles.size());
  for (const auto& kv : simulators) {
    v.emplace_back(kv.first);
  }
  for (const auto& kv : controllers) {
    v.emplace_back(kv.first);
  }
  for (const auto& kv : vehicles) {
    v.emplace_back(kv.first);
  }
  return v;
}

namespace {

template <typename T>
std::vector<std::string> map_keys(const std::map<std::string, T>& xs) {
  std::vector<std::string> v;
  v.reserve(xs.size());
  for (const auto& kv : xs) {
    v.emplace_back(kv.first);
  }
  return v;
}

}  // anonymous namespace

std::vector<std::string> SimulationContext::simulator_ids() const { return map_keys(simulators); }
std::vector<std::string> SimulationContext::controller_ids() const { return map_keys(controllers); }
std::vector<std::string> SimulationContext::vehicle_ids() const { return map_keys(vehicles); }
std::vector<std::string> SimulationContext::plugin_ids() const {
  return map_keys(config.get_all_plugins());
}

std::shared_ptr<cloe::Registrar> SimulationContext::simulation_registrar() {
  if (config.simulation.name) {
    return registrar->with_trigger_prefix(*config.simulation.name);
  } else {
    return registrar;
  }
}

bool SimulationContext::foreach_model(std::function<bool(cloe::Model&, const char*)> f) {
  for (auto& kv : controllers) {
    auto ok = f(*kv.second, "controller");
    if (!ok) return false;
  }
  for (auto& kv : vehicles) {
    auto ok = f(*kv.second, "vehicle");
    if (!ok) return false;
  }
  for (auto& kv : simulators) {
    auto ok = f(*kv.second, "simulator");
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_model(
    std::function<bool(const cloe::Model&, const char*)> f) const {
  for (auto& kv : controllers) {
    auto ok = f(*kv.second, "controller");
    if (!ok) return false;
  }
  for (auto& kv : vehicles) {
    auto ok = f(*kv.second, "vehicle");
    if (!ok) return false;
  }
  for (auto& kv : simulators) {
    auto ok = f(*kv.second, "simulator");
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_simulator(std::function<bool(cloe::Simulator&)> f) {
  for (auto& kv : simulators) {
    auto ok = f(*kv.second);
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_simulator(std::function<bool(const cloe::Simulator&)> f) const {
  for (auto& kv : simulators) {
    auto ok = f(*kv.second);
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_vehicle(std::function<bool(cloe::Vehicle&)> f) {
  for (auto& kv : vehicles) {
    auto ok = f(*kv.second);
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_vehicle(std::function<bool(const cloe::Vehicle&)> f) const {
  for (auto& kv : vehicles) {
    auto ok = f(*kv.second);
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_controller(std::function<bool(cloe::Controller&)> f) {
  for (auto& kv : controllers) {
    auto ok = f(*kv.second);
    if (!ok) return false;
  }
  return true;
}

bool SimulationContext::foreach_controller(std::function<bool(const cloe::Controller&)> f) const {
  for (auto& kv : controllers) {
    auto ok = f(*kv.second);
    if (!ok) return false;
  }
  return true;
}

}  // namespace engine
