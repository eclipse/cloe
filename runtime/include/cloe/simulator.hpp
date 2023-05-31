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
 * \file cloe/simulator.hpp
 * \see  cloe/model.hpp
 *
 * This file provides the base classes for simulator models.
 */

#pragma once

#include <memory>       // for shared_ptr<>, unique_ptr<>, make_unique<>
#include <string>       // for string
#include <type_traits>  // for decay

#include <cloe/model.hpp>  // for Model, ModelFactory

/**
 * This macro defines a SimulatorFactory named xFactoryType and with the
 * configuration xConfigType. It should be used within the desired namespace.
 *
 * The `make` method needs to be implemented:
 *
 * ```
 * std::unique_ptr<Simulator> xFactoryType::make(const Conf& c) {
 *   // implementation
 * }
 * ```
 *
 * The DEFINE_SIMULATOR_FACTORY_MAKE macro can also be used to use the default
 * implementation.
 */
#define DEFINE_SIMULATOR_FACTORY(xFactoryType, xConfigType, xName, xDescription) \
  class xFactoryType : public ::cloe::SimulatorFactory {                         \
   public:                                                                       \
    xFactoryType() : ::cloe::SimulatorFactory(xName, xDescription) {}            \
    std::unique_ptr<::cloe::SimulatorFactory> clone() const override {           \
      return std::make_unique<std::decay<decltype(*this)>::type>(*this);         \
    }                                                                            \
    std::unique_ptr<::cloe::Simulator> make(const ::fable::Conf&) const override; \
                                                                                 \
   protected:                                                                    \
    ::cloe::Schema schema_impl() override { return config_.schema(); }           \
                                                                                 \
   private:                                                                      \
    xConfigType config_;                                                         \
  };

/**
 * This macro defines the xFactoryType::make method.
 *
 * For this to work, the xSimulatorType must have a constructor with the
 * following signature (see DEFINE_SIMULATOR_FACTORY macro):
 *
 *    xSimulatorType(const std::string&, const xConfigType&)
 */
#define DEFINE_SIMULATOR_FACTORY_MAKE(xFactoryType, xSimulatorType)                    \
  std::unique_ptr<::cloe::Simulator> xFactoryType::make(const ::fable::Conf& c) const { \
    decltype(config_) conf{config_};                                                   \
    if (!c->is_null()) {                                                               \
      conf.from_conf(c);                                                               \
    }                                                                                  \
    return std::make_unique<xSimulatorType>(this->name(), conf);                       \
  }

namespace cloe {

// Forward declarations:
class Sync;     // from cloe/sync.hpp
class Vehicle;  // from cloe/vehicle.hpp

/**
 * The Simulator interface provides a model of the world.
 *
 * This class binds Cloe to the a simulator, such as VTD or Minimator.
 * In particular, it...
 *
 * - provides access to available vehicles
 * - updates the vehicle's sensed state and world
 * - sends vehicle actuation to the simulator
 * - keeps the simulator synchronized
 *
 * A Simulator is not expected to survive for more than one simulation.
 * However, it should be able to connect and disconnect to a simulation, so
 * that more than one Simulator instance can exist at any point in time.
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
 * The majority of the step-for-step work occurs in the process method. After
 * each simulator binding has processed, the vehicles will be processed.
 * Since under-the-hood, each vehicle is provided by one of the simulators,
 * consider that vehicle-specific work does not need to be done in the
 * simulator process method.
 *
 * \see  cloe/model.hpp
 *
 * Special Methods
 * ---------------
 * The following methods are specific to the Simulator interface:
 *
 * - `size_t num_vehicles()`
 * - `std::shared_ptr<Vehicle> get_vehicle(size_t)`
 * - `std::shared_ptr<Vehicle> get_vehicle(const std::string&)`
 */
class Simulator : public Model {
 public:
  using Model::Model;
  virtual ~Simulator() noexcept = default;

  /**
   * Return the number of vehicles that the simulator binding has access to.
   *
   * - It may throw an exception or return 0 if the simulator is not connected.
   */
  virtual size_t num_vehicles() const = 0;

  /**
   * Return a pointer to a Vehicle.
   *
   * - The argument i should be between [0, n), where n = num_vehicles().
   * - If a vehicle index is accessed that is out-of-bounds, behaviour is
   *   undefined.
   * - The simulator binding must manage the memory of the Vehicle.
   * - The pointer to the vehicle is guaranteed to be valid for the duration of
   *   the simulation.
   * - After the simulation has disconnected, the pointer is no longer
   *   guaranteed to be valid.
   * - The vehicle may be modified and these modifications will be preserved
   *   for the validity of the pointer. In particular, the sensor and actuator
   *   interfaces may be replaced by proxies.
   */
  virtual std::shared_ptr<Vehicle> get_vehicle(size_t i) const = 0;

  /**
   * Return a pointer to a Vehicle.
   *
   * - If a vehicle does not exist with the key, nullptr is returned.
   * - Same conditions apply as for get_vehicle(size_t).
   */
  virtual std::shared_ptr<Vehicle> get_vehicle(const std::string& key) const = 0;

  /**
   * Send vehicle actuations to the simulator and retrieve the new world state.
   *
   * 1. For each vehicle, actuation is sent first, to make clear that these
   *    actuations are based on the sensor information from the previous step.
   *    The new world state, at the time point where the actuations are sent is
   *    then retrieved and made available to each vehicle.  Essentially, they
   *    are sent and received in parallel.
   * 2. The simulator is instructed to complete the next simulation cycle, if
   *    possible asynchronously.
   *
   * - This function can throw an exception, which causes the simulation to be
   *   aborted.
   *
   * \see Model::process
   */
  Duration process(const Sync&) override = 0;
};

/**
 * A SimulatorFactory creates a new Simulator and is required for each
 * Simulator implementation.
 */
class SimulatorFactory : public ModelFactory {
 public:
  static constexpr char const* PLUGIN_TYPE = "simulator";
  static constexpr char const* PLUGIN_API_VERSION = "2.0";

  /**
   * Creates a new base instance of a factory.
   */
  using ModelFactory::ModelFactory;
  virtual ~SimulatorFactory() noexcept = default;

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
  virtual std::unique_ptr<SimulatorFactory> clone() const = 0;

  /**
   * Create a new Simulator based on the current configuration and the given
   * Conf.
   *
   * - This method may throw Error.
   */
  virtual std::unique_ptr<Simulator> make(const fable::Conf& c) const = 0;
};

}  // namespace cloe
