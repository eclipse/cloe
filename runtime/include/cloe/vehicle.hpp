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
 * \file cloe/vehicle.hpp
 * \see  cloe/vehicle.cpp
 *
 * This file contains essential definitions for the Vehicle class.
 */

#pragma once
#ifndef CLOE_VEHICLE_HPP_
#define CLOE_VEHICLE_HPP_

#include <cstdint>      // for uint64_t
#include <map>          // for map<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_enum<>

#include <cloe/component.hpp>  // for Component
#include <cloe/core.hpp>       // for Json
#include <cloe/model.hpp>      // for Model

namespace cloe {

/**
 * A Vehicle is a collection of sensor and actuator components.
 *
 * Some of the components are sensors, and some are actuators. From the
 * perspective of a controller, a sensor can be thought of as a read-only
 * device, while an actuator can be thought of as a read-write device. The
 * simulator will have a reversed perspectice, as it supplies data to or
 * through sensor components and reads from actuators.
 *
 * The initial set of components that are part of a Vehicle are supplied by a
 * simulator. A simulator binding may derive from the Vehicle class and provide
 * its own components that are (encouraged to derive from Cloe standard)
 * sensors and actuators.
 *
 * Through runtime configuration, the initial set of components may be extended
 * or modified. It is therefore important that simulator bindings maintain
 * an internal list of components which are read and written.
 */
class Vehicle : public Model {
 public:
  Vehicle(uint64_t id, const std::string& name) : Model(name), id_(id) {}
  virtual ~Vehicle() noexcept = default;

  /**
   * Return a clone of a vehicle with the given ID and name.
   *
   * This retains all the components that the vehicle has and allows the cloned
   * vehicle to be modified without affecting the original vehicle.
   */
  std::shared_ptr<Vehicle> clone(uint64_t id, const std::string& name);

  uint64_t id() const { return id_; }

  /**
   * Return the number of components in the vehicle.
   */
  size_t size() const { return this->components_.size(); }

  /**
   * Return whether the vehicle has a component with the given name.
   */
  bool has(const std::string& key) const { return this->components_.count(key) != 0; }

  /**
   * Return whether the vehicle has the component as identified by the enum
   * value.
   */
  template <typename Enum, std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
  bool has(Enum c) const {
    return this->has(to_string(c));
  }

  /**
   * Return the component with the given key if it exists.
   *
   * This may throw std::out_of_range.
   */
  template <typename T>
  std::shared_ptr<T> get(const std::string& key) {
    return std::dynamic_pointer_cast<T>(at(key));
  }

  /**
   * Return the component associated with the standard Cloe enum value.
   *
   * Under-the-hood, the enum is translated to a string, which is used to fetch
   * the correct component.
   *
   * This may throw std::out_of_range.
   */
  template <typename T, typename Enum, std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
  std::shared_ptr<T> get(Enum c) {
    return std::dynamic_pointer_cast<T>(at(to_string(c)));
  }

  template <typename T>
  std::shared_ptr<const T> get(const std::string& key) const {
    return std::dynamic_pointer_cast<const T>(at(key));
  }

  template <typename T, typename Enum, std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
  std::shared_ptr<const T> get(Enum c) const {
    return std::dynamic_pointer_cast<T>(at(to_string(c)));
  }

 public:  // Component Management
  template <typename... Arguments>
  void new_component(Component* ptr, const Arguments&... aliases) {
    auto sp = std::shared_ptr<Component>(ptr);
    this->add_component(sp, aliases...);
  }

  template <typename First, typename... Arguments>
  void add_component(std::shared_ptr<Component> sp, const First& alias,
                     const Arguments&... aliases) {
    this->add_component(sp, alias);
    this->add_component(sp, aliases...);
  }

  template <typename Enum, std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
  void add_component(std::shared_ptr<Component> sp, Enum c) {
    this->add_component(sp, to_string(c));
  }

  void add_component(std::shared_ptr<Component> sp, const std::string& alias) {
    if (this->has(alias)) {
      // TODO(ben): Add better error type here with explanation
      throw std::runtime_error("component already exists in the vehicle");
    }
    this->set_component(alias, sp);
  }

  template <typename First, typename... Arguments>
  void emplace_component(std::shared_ptr<Component> sp, const First& alias,
                         const Arguments&... aliases) {
    this->emplace_component(sp, alias);
    this->emplace_component(sp, aliases...);
  }

  template <typename Enum, std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
  void emplace_component(std::shared_ptr<Component> sp, Enum c) {
    this->emplace_component(sp, to_string(c));
  }

  void emplace_component(std::shared_ptr<Component> sp, const std::string& alias) {
    this->set_component(alias, sp);
  }

  void set_component(const std::string& key, std::shared_ptr<Component> component) {
    this->components_[key] = component;
  }

 public:  // Overrides
  /**
   * Process all components.
   *
   * This primarily consists of clearing the cache and updating internal state.
   *
   * # Note
   *
   * This may occur multiple times for each component, even if a component only
   * exists once in a vehicle. For example, if a component exists in a vehicle,
   * but then is wrapped by two different filters; the original sensor may not
   * be directly reachable, but two different components will update it.
   */
  Duration process(const Sync& sync) override;
  void connect() override;
  void disconnect() override;

  void enroll(Registrar& r) override;
  void reset() override;
  void abort() override;

  friend void to_json(Json& j, const Vehicle& v) {
    j = Json{
        {"id", v.id()},
        {"name", v.name()},
        {"components", v.components_},
    };
  }

 protected:  // Useful methods
  /**
   * Return the component with the given name or throw an Error that is
   * actually helpful.
   */
  std::shared_ptr<const Component> at(const std::string& key) const;
  std::shared_ptr<Component> at(const std::string& key);

 private:
  uint64_t id_;

  /**
   * Components are stored in a hash map, accessible by string.
   *
   * # Rationale
   *
   * If we knew the breadth of components that could be part of a vehicle, we
   * could make the key of the map an enum, which is desirable for performance
   * and clarity reasons.
   *
   * The problem is that even if we have an enum, it is not clear what value to
   * give a sensor that is created at runtime and does not fit into the
   * existing scheme.
   *
   * Components are only possibly owned by a vehicle. maybe_own is a shared_ptr
   * that may have a deleter that does nothing. It might be better to simplify
   * this to a shared_ptr and deal with the collateral damage.
   */
  std::map<std::string, std::shared_ptr<Component>> components_;
};

/**
 * This error is thrown when an unknown component is accessed at a Vehicle.
 */
class UnknownComponentError : public Error {
 public:
  UnknownComponentError(const std::string& vehicle,
                        const std::string& key,
                        const std::vector<std::string>& available_components);

  const std::string& vehicle() const { return vehicle_; }
  const std::string& unknown_component() const { return unknown_; }
  const std::vector<std::string>& available_components() const;

 private:
  std::string vehicle_;
  std::string unknown_;
  std::vector<std::string> available_;
};

}  // namespace cloe

#endif  // CLOE_VEHICLE_HPP_
