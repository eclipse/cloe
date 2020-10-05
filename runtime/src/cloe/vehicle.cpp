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
 * \file cloe/vehicle.cpp
 * \see  cloe/vehicle.hpp
 */

#include <cloe/vehicle.hpp>

#include <cstdint>  // for uint64_t
#include <map>      // for map<>
#include <memory>   // for shared_ptr<>
#include <set>      // for set<>
#include <string>   // for string

#include <cloe/registrar.hpp>  // for Registrar
#include <cloe/sync.hpp>       // for Sync

#include "cloe/utility/std_extensions.hpp"  // for join_vector, map_keys

namespace cloe {

std::shared_ptr<Vehicle> Vehicle::clone(uint64_t id, const std::string& name) {
  auto veh = std::make_shared<Vehicle>(id, name);
  veh->components_ = std::map<std::string, std::shared_ptr<Component>>(components_);
  return veh;
}

void Vehicle::enroll(Registrar& r) {
  r.register_api_handler("/", HandlerType::BUFFERED, handler::ToJson<Vehicle>(this));
}

void Vehicle::connect() {
  for (auto& kv : components_) {
    // A component may proxy another or be available by an alias,
    // thus we only connect those that are not already connected.
    if (!kv.second->is_connected()) {
      kv.second->connect();
    }
  }
  Model::connect();
}

void Vehicle::disconnect() {
  for (auto& kv : components_) {
    // A component may proxy another or be available by an alias,
    // thus we only disconnect those that are not already connected.
    if (kv.second->is_connected()) {
      kv.second->disconnect();
    }
  }
  Model::disconnect();
}

Duration Vehicle::process(const Sync& sync) {
  std::set<uint64_t> component_ids;
  Duration target = sync.time();
  for (auto& c : this->components_) {
    auto id = c.second->id();
    if (component_ids.count(id) == 0) {
      Duration t = c.second->process(sync);
      if (t < target) {
        return t;
      }
      component_ids.insert(id);
    }
  }
  return target;
}

void Vehicle::reset() {
  for (auto& c : this->components_) {
    c.second->reset();
  }
}

void Vehicle::abort() {
  for (auto& c : this->components_) {
    c.second->abort();
  }
}

std::shared_ptr<const Component> Vehicle::at(const std::string& key) const {
  try {
    return components_.at(key);
  } catch (std::out_of_range& e) {
    throw UnknownComponentError(name(), key, utility::map_keys(components_));
  }
}

std::shared_ptr<Component> Vehicle::at(const std::string& key) {
  return std::const_pointer_cast<Component>(const_cast<const Vehicle&>(*this).at(key));
}

UnknownComponentError::UnknownComponentError(const std::string& vehicle,
                                             const std::string& key,
                                             const std::vector<std::string>& available_components)
    : Error("vehicle {}: no component available with name: {}", vehicle, key)
    , vehicle_(vehicle)
    , unknown_(key)
    , available_(available_components) {
  // clang-format off
  this->set_explanation(R"(
  It looks like you are trying to access a component on the vehicle that does
  not exist:

      {}

  This error can have several causes, depending on what you are
  trying to achieve and where the component is supposed to come from:

    a) Use one of the components already in the vehicle:

         {}

    b) Add the component to the vehicle, by:
       i) configuring it in the stackfile at /vehicles/<index>/components/<key>, or
       ii) adding it to the vehicle in the simulator binding plugin.

  Note: You can also use the web API to inspect a vehicle during runtime.
  )", key, utility::join_vector(available_, "\n         "));
  // clang-format on
}

}  // namespace cloe
