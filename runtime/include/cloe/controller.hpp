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
 * \file cloe/controller.hpp
 * \see  cloe/model.hpp
 *
 * This file provides the base classes for controller models.
 */

#pragma once

#include <memory>       // for shared_ptr<>, unique_ptr<>, make_unique<>
#include <type_traits>  // for decay
#include <utility>      // for move

#include <cloe/model.hpp>  // for Model, ModelFactory

/**
 * This macro defines a ControllerFactory named xFactoryType and with the
 * configuration xConfigType. It should be used within the desired namespace.
 *
 * The `make` method needs to be implemented:
 *
 * ```
 * std::unique_ptr<Controller> xFactoryType::make(const Conf& c) {
 *   // implementation
 * }
 * ```
 *
 * The DEFINE_CONTROLLER_FACTORY_MAKE macro can also be used to use the default
 * implementation.
 */
#define DEFINE_CONTROLLER_FACTORY(xFactoryType, xConfigType, xName, xDescription) \
  class xFactoryType : public ::cloe::ControllerFactory {                         \
   public:                                                                        \
    xFactoryType() : ::cloe::ControllerFactory(xName, xDescription) {}            \
    std::unique_ptr<::cloe::ControllerFactory> clone() const override {           \
      return std::make_unique<std::decay<decltype(*this)>::type>(*this);          \
    }                                                                             \
                                                                                  \
    std::unique_ptr<::cloe::Controller> make(const ::fable::Conf&) const override; \
                                                                                  \
   protected:                                                                     \
    ::cloe::Schema schema_impl() override { return config_.schema(); }            \
                                                                                  \
   private:                                                                       \
    xConfigType config_;                                                          \
  };

/**
 * This macro defines the xFactoryType::make method.
 *
 * For this to work, the xControllerType must have a constructor with the
 * following signature (see DEFINE_CONTROLLER_FACTORY macro):
 *
 *    xControllerType(const std::string&, const xConfigType&)
 */
#define DEFINE_CONTROLLER_FACTORY_MAKE(xFactoryType, xControllerType)                   \
  std::unique_ptr<::cloe::Controller> xFactoryType::make(const ::fable::Conf& c) const { \
    decltype(config_) conf{config_};                                                    \
    if (!c->is_null()) {                                                                \
      conf.from_conf(c);                                                                \
    }                                                                                   \
    return std::make_unique<xControllerType>(this->name(), conf);                       \
  }

namespace cloe {

// Forward declarations:
class Vehicle;  // from cloe/vehicle.hpp

/**
 * The Controller class serves as an interface which every controller binding
 * should inherit from.
 *
 * It differs from its base class `Model` by providing a method to assign a
 * `Vehicle` to it. Thus, a controller is bound to a single vehicle, which
 * it can use during processing for input and output purposes.
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
 * The following methods are specific to the Controller interface:
 *
 * - `bool has_vehicle()`
 * - `std::shared_ptr<Vehicle> get_vehicle()`
 * - `void set_vehicle(std::shared_ptr<Vehicle>)`
 */
class Controller : public Model {
 public:
  using Model::Model;
  virtual ~Controller() noexcept = default;

  /**
   * Return whether the controller has a vehicle assigned to it.
   *
   * This is a prerequisite for the controller to be able to run.
   */
  virtual bool has_vehicle() const { return static_cast<bool>(veh_); }

  /**
   * Return a pointer to the vehicle that is assigned to the controller.
   *
   * - If nullptr is returned, this indicates that the controller is unpaired
   *   and disabled.
   */
  virtual std::shared_ptr<Vehicle> get_vehicle() const { return veh_; }

  /**
   * Assign a vehicle to the controller.
   *
   * - If v == nullptr, then the controller's process method will not be called.
   * - If called multiple times, the last call overrides any previous ones.
   * - The controller is not responsible for the deletion of the vehicle.
   * - The pointer passed in is valid for the duration of the simulation.
   */
  virtual void set_vehicle(std::shared_ptr<Vehicle> v) { veh_ = std::move(v); }

 protected:
  std::shared_ptr<Vehicle> veh_{nullptr};
};

/**
 * A ControllerFactory creates a new Controller and is required for each
 * Controller implementation.
 */
class ControllerFactory : public ModelFactory {
 public:
  static constexpr char const* PLUGIN_TYPE = "controller";
  static constexpr char const* PLUGIN_API_VERSION = "2.0";

  /**
   * Creates a new base instance of a factory.
   */
  using ModelFactory::ModelFactory;
  virtual ~ControllerFactory() noexcept = default;

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
  virtual std::unique_ptr<ControllerFactory> clone() const = 0;

  /**
   * Create a new Controller based on the current configuration and the given
   * Conf.
   *
   * - This method may throw Error.
   */
  virtual std::unique_ptr<Controller> make(const fable::Conf& c) const = 0;
};

}  // namespace cloe
