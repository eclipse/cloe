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
 * \file cloe/component.hpp
 * \see  cloe/component.cpp
 * \see  cloe/model.hpp
 * \see  cloe/vehicle.hpp
 *
 * This file provides the base classes for component models.
 */

#pragma once

#include <cstdint>      // for uint64_t
#include <memory>       // for shared_ptr<>, unique_ptr<>, make_unique<>
#include <string>       // for string
#include <type_traits>  // for decay
#include <utility>      // for move

#include <fable/fable_fwd.hpp>  // for Json

#include <cloe/model.hpp>  // for Model, ModelFactory
#include <cloe/sync.hpp>   // for Sync

/**
 * This macro defines a ComponentFactory named xFactoryType and with the
 * configuration xConfigType. It should be used within the desired namespace.
 *
 * The `make` method needs to be implemented:
 *
 * ```
 * std::unique_ptr<Component> xFactoryType::make(const Conf& c) {
 *   // implementation
 * }
 * ```
 *
 * The DEFINE_COMPONENT_FACTORY_MAKE macro can also be used to use the default
 * implementation.
 */
#define DEFINE_COMPONENT_FACTORY(xFactoryType, xConfigType, xName, xDescription)              \
  class xFactoryType : public ::cloe::ComponentFactory {                                      \
   public:                                                                                    \
    xFactoryType() : ComponentFactory(xName, xDescription) {}                                 \
    std::unique_ptr<::cloe::ComponentFactory> clone() const override {                        \
      return std::make_unique<std::decay<decltype(*this)>::type>(*this);                      \
    }                                                                                         \
    std::unique_ptr<::cloe::Component> make(                                                  \
        const ::fable::Conf&, std::vector<std::shared_ptr<::cloe::Component>>) const override; \
                                                                                              \
   protected:                                                                                 \
    ::cloe::Schema schema_impl() override { return config_.schema(); }                        \
                                                                                              \
   private:                                                                                   \
    xConfigType config_;                                                                      \
  };

/**
 * This macro defines the xFactoryType::make method for components with exactly
 * one input component.
 *
 * For this to work, the xComponentType must have a constructor with the
 * following signature (see DEFINE_COMPONENT_FACTORY macro):
 *
 *    xComponentType(const std::string&, const xConfigType&, std::shared_ptr<xInputType>)
 */
#define DEFINE_COMPONENT_FACTORY_MAKE(xFactoryType, xComponentType, xInputType)              \
  std::unique_ptr<::cloe::Component> xFactoryType::make(                                     \
      const ::fable::Conf& c, std::vector<std::shared_ptr<::cloe::Component>> comp) const {   \
    decltype(config_) conf{config_};                                                         \
    assert(comp.size() == 1);                                                                \
    if (!c->is_null()) {                                                                     \
      conf.from_conf(c);                                                                     \
    }                                                                                        \
    return std::make_unique<xComponentType>(                                                 \
        this->name(), conf, std::dynamic_pointer_cast<xInputType>(std::move(comp.front()))); \
  }

namespace cloe {

/**
 * A Component is a sensor or actuator that is part of a vehicle.
 * Components can be used by controllers for reading and writing.
 *
 * Generally, one does not directly inherit from Component when creating a new
 * component plugin. Instead, an intermediate interface is created, such as
 * `EgoSensor`, which is then used as the base class. This allows multiple
 * implementations and proxies to augment components transparently.
 * As such, the interface provided by the Component class is quite slim.
 *
 * Note: If you are creating a Component and it is not a standard CloeComponent,
 * it is recommended to create a static method
 *
 *    static const char* default_name()
 *
 * that returns a unique identifier (e.g. with namespace and so on).
 *
 * Inherited Methods
 * -----------------
 * Make sure to implement the following methods from the Model interface:
 *
 * - `Duration resolution() const`
 * - `bool is_connected() const`
 * - `bool is_operational() const`
 * - `void connect()`
 * - `void disconnect()`
 * - `void enroll(Registrar&)`
 * - `void start(const Sync&)`
 * - `Duration process(const Sync&) = 0`
 * - `void pause(const Sync&)`
 * - `void resume(const Sync&)`
 * - `void stop(const Sync&)`
 * - `void reset()`
 * - `void abort()`
 *
 * See the documentation for the `Model` class for information on when these
 * methods are called in a simulation.
 *
 * \see  cloe/model.hpp
 *
 * Special Methods
 * ---------------
 * The following methods are specific to the Component interface:
 *
 * - `uint64_t id() const`
 * - `T* as()`
 * - `fable::Json active_state() const = 0`
 */
class Component : public Model {
 public:
  explicit Component(const std::string& name, const std::string& description = "")
      : Model(name, description), id_(++gid_) {}
  explicit Component(std::string&& name, std::string&& description = "")
      : Model(std::move(name), std::move(description)), id_(++gid_) {}
  virtual ~Component() noexcept = default;

  /**
   * Return the unique numeric ID of this component.
   *
   * This numeric ID should be unique across all components in a simulation.
   * There is no guarantee however, that the component will receive the same ID
   * in any future simulation. Numeric IDs start with 1; 0 is not a valid ID.
   */
  uint64_t id() const { return id_; }

  /**
   * Attempt to cast this component to a sub-type.
   *
   * - Throws an exception if the component cannot be cast.
   */
  template <typename T>
  T* as() {
    return dynamic_cast<T*>(this);
  }

  /**
   * Return the JSON representation of the component.
   */
  virtual fable::Json active_state() const = 0;

  /**
   * Clear any cache that may be accumulated during a step.
   *
   * - Each inheriting class should call the super class.
   * - This may be called multiple times.
   */
  Duration process(const Sync& sync) override { return sync.time(); }

  /**
   * Implement Model::reset by default.
   */
  void reset() override {}

  /**
   * Implement Model::abort by default.
   */
  void abort() override {}

 protected:
  friend void to_json(fable::Json& j, const Component& c) {
    j = c.active_state();
    j["id"] = c.id();
    j["name"] = c.name();
  }

 private:
  static uint64_t gid_;
  uint64_t id_;
};

/**
 * A ComponentFactory creates a new Component and is required for each
 * Component implementation that is to be user-configurable.
 */
class ComponentFactory : public ModelFactory {
 public:
  static constexpr char const* PLUGIN_TYPE = "component";
  static constexpr char const* PLUGIN_API_VERSION = "2.0";

  /**
   * Creates a new base instance of a factory.
   */
  using ModelFactory::ModelFactory;
  virtual ~ComponentFactory() noexcept = default;

  /**
   * Create a clone of the factory with its current configuration.
   *
   * This cannot be done from the abstract class, but the implementation in
   * most inheriting classes can be a oneliner:
   *
   *     return std::make_unique<std::decay<decltype(*this)>::type>(*this);
   *
   * The above one-liner is only required for genericity and is equivalent to:
   *
   *     return std::make_unique<FoobarFactory>(*this);
   */
  virtual std::unique_ptr<ComponentFactory> clone() const = 0;

  /**
   * Create a new Component based on the current configuration and the given
   * Conf.
   *
   * - This method may throw Error.
   */
  virtual std::unique_ptr<Component> make(const fable::Conf& c,
                                          std::vector<std::shared_ptr<Component>>) const = 0;
};

}  // namespace cloe
