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
 * \file cloe/model.hpp
 * \see  cloe/simulator.hpp
 * \see  cloe/controller.hpp
 * \see  cloe/component.hpp
 *
 * This file provides several base classes for models or agents participating
 * in a simulation.
 *
 * Model
 *   Base class for Simulator, Controller, and Component.
 *
 * ModelFactory
 *   Base class for SimulatorFactory, ControllerFactory, and ComponentFactory.
 *
 * ModelError
 *   Base class for all runtime errors occurring during model connection or
 *   processing.
 *
 * ModelAbort
 *   Error used when a model needs to *signal* simulation abortion.
 *
 * ModelReset
 *   Error used when a model needs to *signal* that it desires a simulation
 *   reset.
 *
 * ModelStop
 *   Error used when a model needs to *signal* that it desires a simulation
 *   stop.
 */

#pragma once

#include <fable/confable.hpp> // for Confable

#include <cloe/core.hpp>    // for Duration, Error
#include <cloe/entity.hpp>  // for Entity

namespace cloe {

// Forward declaration:
class Sync;       // from cloe/sync.hpp
class Registrar;  // from cloe/registrar.hpp

/**
 * ModelError indicates that an error in a model has occurred.
 */
class ModelError : public Error {
 public:
  using Error::Error;
  virtual ~ModelError() noexcept = default;

  const std::string& explanation() const { return Error::explanation(); }

  ModelError explanation(const std::string& explanation) && {
    this->set_explanation(explanation);
    return std::move(*this);
  }

  template <typename... Args>
  ModelError explanation(std::string_view format, const Args&... args) && {
    this->set_explanation(fmt::format(fmt::runtime(format), args...));
    return std::move(*this);
  }
};

/**
 * ModelAbort indicates that the model has encountered an error or received a
 * request that causes it to believe that the simulation should be aborted.
 *
 * \see  AsyncAbort
 */
class ModelAbort : public ModelError {
 public:
  using ModelError::ModelError;
  virtual ~ModelAbort() noexcept = default;

  const std::string& explanation() const { return Error::explanation(); }

  ModelError explanation(const std::string& explanation) && {
    this->set_explanation(explanation);
    return std::move(*this);
  }

  template <typename... Args>
  ModelAbort explanation(std::string_view format, const Args&... args) && {
    this->set_explanation(fmt::format(fmt::runtime(format), args...));
    return std::move(*this);
  }
};

/**
 * ModelReset indicates that the model has encountered a request that causes
 * it to believe that the simulation should be restarted.
 */
class ModelReset : public ModelError {
 public:
  using ModelError::ModelError;
  virtual ~ModelReset() noexcept = default;

  const std::string& explanation() const { return Error::explanation(); }

  ModelError explanation(const std::string& explanation) && {
    this->set_explanation(explanation);
    return std::move(*this);
  }

  template <typename... Args>
  ModelReset explanation(std::string_view format, const Args&... args) && {
    this->set_explanation(fmt::format(fmt::runtime(format), args...));
    return std::move(*this);
  }
};

/**
 * ModelStop indicates that the model has encountered a request that causes
 * it to believe that the simulation should be stopped.
 */
class ModelStop : public ModelError {
 public:
  using ModelError::ModelError;
  virtual ~ModelStop() noexcept = default;

  const std::string& explanation() const { return Error::explanation(); }

  ModelError explanation(const std::string& explanation) && {
    this->set_explanation(explanation);
    return std::move(*this);
  }

  template <typename... Args>
  ModelStop explanation(std::string_view format, const Args&... args) && {
    this->set_explanation(fmt::format(fmt::runtime(format), args...));
    return std::move(*this);
  }
};

/**
 * The Model class serves as an interface which Controller and Simulator can
 * inherit from.
 *
 * The following flow diagram shows how the methods of a Model are called in
 * a typical simulation. The nominal flow is rendered in solid lines, while
 * irregular situations are rendered in dashed lines.
 *
 *                                ┌──────────────────────┐
 *                                │       Model()        │
 *                                └──────────────────────┘
 *                                           │
 *                                           ▼
 *                                ┌──────────────────────┐
 *      +------------------------ │      connect()       │
 *      |                         └──────────────────────┘
 *      |                                    │
 *      |                                    ▼
 *      |                         ┌──────────────────────┐
 *      |                         │  enroll(Registrar&)  │
 *      |                         └──────────────────────┘
 *      |                                    │
 *      |                                    ▼
 *      |                         ┌──────────────────────┐
 *      |                         │  start(const Sync&)  │ <-----------+
 *      |                         └──────────────────────┘             |
 *      |  +---------------+                 │                         |
 *      |  |  resume(...)  | ----------+     │                         |
 *      |  +---------------+           |     │                         |
 *      |          ^                   v     ▼                         |
 *      |          |              ┌──────────────────────┐             |
 *      |  +---------------+      │                      │ ◀──┐        |
 *      |  |   pause(...)  | <--- │                      │    │        |
 *      |  +---------------+      │ process(const Sync&) │    │        |
 *      |        |                │                      │    │        |
 *      |        |     +--------- │                      │ ───┘        |
 *      |        |     |          └──────────────────────┘             |
 *      |        |     |                     │                         |
 *      |        v     v                     ▼                         |
 *      |     +-----------+       ┌──────────────────────┐       +-----------+
 *      +---> |  abort()  | ----> │  stop(const Sync&)   │ ----> |  reset()  |
 *            +-----------+       └──────────────────────┘       +-----------+
 *                  |                        │
 *                  |                        ▼
 *                  |             ┌──────────────────────┐
 *                  +-----------> │     disconnect()     │
 *                                └──────────────────────┘
 *                                           │
 *                                           ▼
 *                                ┌──────────────────────┐
 *                                │      ~Model()        │
 *                                └──────────────────────┘
 *
 * Note that not all of the methods of a Model are presented in the above
 * diagram. For example, the methods `resolution()` and `is_connected()` are
 * expected to be callable at all times. The edges present cover roughly 95%
 * of all expected use-cases, but not all. For example, it is conceivable that
 * a Model will be destructed before `connect()` is called.
 */
class Model : public Entity {
 public:
  /**
   * Construct a model.
   *
   * Construction must specify a name and optionally a description.
   * The name must adhere to the regulations laid down in the Entity class.
   *
   * A model should be considered disconnected after construction, that is
   * `is_connected()` should return false.
   */
  using Entity::Entity;

  /**
   * Destroy a model.
   *
   * Destruction must handle everything that must be torn down after
   * a disconnect or possibly even before a connect has occurred.
   */
  virtual ~Model() noexcept = default;

  /**
   * Return the time resolution of the model.
   *
   * This is how much time each process call is expected to advance.
   * Zero may be returned if the model does not have an intrinsic time
   * resolution. (This would imply that calling `process(const Sync&)`
   * will advance the models time to exactly `Sync::time()`.
   */
  virtual Duration resolution() const;

  /**
   * Return whether the model is successfully connected.
   */
  virtual bool is_connected() const { return connected_; }

  /**
   * Return whether the model can continue processing.
   */
  virtual bool is_operational() const { return operational_; }

  /**
   * Initiate a connection to the model, including any initialization.
   *
   * - It should block until the connection is successfully established.
   * - It should throw an exception if an unrecoverable error occurs.
   * - It should react to an abort() called on the model.
   * - It may throw an error if already connected.
   *
   * After a successful connection, the following methods shall be callable:
   *
   * - `enroll(Registrar&)`
   * - `start(const Sync&)`
   *
   * When implementing this method, e.g. via override or final, make sure to
   * call this method:
   *
   *    void MySuperModel::connect() final {
   *      // Your connection code
   *      Model::connect();
   *    }
   */
  virtual void connect() { connected_ = true; }

  /**
   * Tear down the connection to the model.
   *
   * - It should block until the connection is successfully closed.
   * - It should throw an exception if an unrecoverable error occurs.
   * - It should not throw an exception if called when not connected.
   *
   * After successful disconnection, it is not expected that the model should
   * be able to connect again.
   *
   * When implementing this method, e.g. via override or final, make sure to
   * call this method:
   *
   *    void MySuperModel::disconnect() final {
   *      // Your disconnection code
   *      Model::disconnect();
   *    }
   */
  virtual void disconnect() { connected_ = false; }

  /**
   * Register any events, actions, or handlers with the registrar.
   *
   * - This may exhibit different behavior when called in a disconnected state,
   *   in order to provide the possibility for verification of triggers in
   *   offline mode.
   * - It may throw an exception.
   */
  virtual void enroll(Registrar&) {}

  /**
   * Perform model setup for the simulation.
   *
   * The passed `Sync` interface should return a step and time of zero.
   *
   * - This will be called once per simulation, before `process(const Sync&)`
   *   is called.
   * - It may throw an exception.
   * - Method `is_operational()` should return true after this.
   */
  virtual void start(const Sync&) { operational_ = true; }

  /**
   * Perform model processing given the simulation context.
   *
   * In particular, the model may read and write information from and to data
   * it has, in particular any vehicles.
   *
   * - Method `is_operational()` should return true.
   * - It may throw an exception. Those derived from ModelError are more likely
   *   to be correctly handled, however. The simulation may handle it by
   *     a) removing or replacing the model,
   *     b) stopping the simulation, or
   *     c) ignoring the exception.
   *   In the last case, this method will be called again in the next
   *   simulation step.  In any case, assume that the simulation will print
   *   a message to the logs or the console.
   * - It should return the current simulation duration from the model's
   *   time domain, which is expected to be less or equal to `sync.time()`.
   */
  virtual Duration process(const Sync&) = 0;

  /**
   * Perform any work for transitioning into a paused state.
   *
   * The passed `Sync` interface should return the step and time that will be
   * passed to the next resume and process calls.
   *
   * - It may throw an exception if the model does not support pausing.
   *   This could be the case if the model is connected to a HIL, for example.
   */
  virtual void pause(const Sync&) {}

  /**
   * Perform any work when resuming from a paused state.
   *
   * The passed `Sync` interface should return the step and time that will be
   * passed to the next process call.
   *
   * - It may throw an exception if the model cannot resume.
   *   This could be the case if the paused timespan was too large so that the
   *   backend died.
   */
  virtual void resume(const Sync&) {}

  /**
   * Perform final work that may throw an exception.
   *
   * This is called after the last process call when the simulation is through.
   * It is possible after a call to stop, that a reset will be called, followed
   * by a further start. Nominally, however, it will be followed by
   * a disconnect.
   *
   * - Method `is_operational()` should return false after this.
   */
  virtual void stop(const Sync&) { operational_ = false; }

  /**
   * Reset the model state.
   *
   * This is called when Cloe is asked to reset the whole simulation to time 0.
   * This can be the case when the simulator or a controller asks us to
   * recover from a temporary problem without repeating the whole simulation
   * setup.
   *
   * The default implementation will raise an error. So if your model is
   * not able to re-initialize, the simulation will be aborted.
   */
  virtual void reset() { throw ModelError("reset not supported by this model"); }

  /**
   * Signal an abort to model processing.
   *
   * This method is called asynchronously. It is highly recommended to make
   * use of a `std::atomic_bool` for purposes of making the abortion request
   * known to other parts of the model. This method is called when the user
   * requests the simulation to be aborted, e.g., by sending the SIGINT signal.
   *
   * An abort will be followed by a stop if the simulation was started.
   *
   * The default implementation will throw an error. This will be caught and
   * possibly ignored. Otherwise, the simulation will be killed.
   *
   * \see  cloe/core/abort.hpp
   */
  virtual void abort() { throw ModelError("abort not supported by this model"); }

 protected:
  bool connected_{false};
  bool operational_{false};
};

/**
 * The ModelFactory class serves as a base class for all other factory classes
 * that make models.
 */
class ModelFactory : public Entity, public fable::Confable {
 public:
  using Entity::Entity;
  virtual ~ModelFactory() = default;
};

}  // namespace cloe
