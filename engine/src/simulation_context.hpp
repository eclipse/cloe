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
 * \file simulation_context.hpp
 * \see  simulation_context.cpp
 */

#pragma once

#include <functional>  // for function<>
#include <map>         // for map<>
#include <memory>      // for unique_ptr<>, shared_ptr<>
#include <optional>    // for optional<>
#include <string>      // for string
#include <vector>      // for vector<>

#include <sol/state_view.hpp>  // for state_view

#include <cloe/cloe_fwd.hpp>       // for Simulator, Controller, Registrar, Vehicle, Duration
#include <cloe/stack.hpp>          // for Stack
#include <cloe/utility/timer.hpp>  // for DurationTimer

#include "simulation_events.hpp"      // for LoopCallback, ...
#include "simulation_outcome.hpp"     // for SimulationOutcome
#include "simulation_probe.hpp"       // for SimulationProbe
#include "simulation_progress.hpp"    // for SimulationProgress
#include "simulation_result.hpp"      // for SimulationResult
#include "simulation_statistics.hpp"  // for SimulationStatistics
#include "simulation_sync.hpp"        // for SimulationSync

namespace engine {

// Forward-declarations:
class CommandExecuter;
class Registrar;
class Coordinator;
class Server;
class SimulationResult;
class SimulationProbe;

/**
 * SimulationContext represents the entire context of a running simulation
 * and is used by SimulationMachine class as the data context for the
 * state machine.
 *
 * The simulation states need to store any data they want to access in the
 * context here. This does have the caveat that all the data here is
 * accessible to all states.
 *
 * All input to and output from the simulation is via this struct.
 */
struct SimulationContext {
  SimulationContext(cloe::Stack conf, sol::state_view l);

  // Configuration -----------------------------------------------------------
  //
  // These values are meant to be set before starting the simulation in order
  // to affect how the simulation is run.
  //
  // The other values in this struct should not be directly modified unless
  // you really know what you are doing.
  //

  cloe::Stack config;  ///< Input configuration.
  std::string uuid{};  ///< UUID to use for simulation.

  /// Report simulation progress to the console.
  bool report_progress{false};

  /// Setup simulation but only probe for information.
  /// The simulation should only go through the CONNECT -> PROBE -> DISCONNECT
  /// state. The same errors that can occur for a normal simulation can occur
  /// here though, so make sure they are handled.
  bool probe_simulation{false};

  // Setup -------------------------------------------------------------------
  //
  // These are functional parts of the simulation framework that mostly come
  // from the engine. They are all initialized in the constructor.
  //
  sol::state_view lua;
  std::unique_ptr<cloe::DataBroker> db;
  std::unique_ptr<Server> server;
  std::shared_ptr<Coordinator> coordinator;
  std::shared_ptr<Registrar> registrar;

  /// Configurable system command executer for triggers.
  std::unique_ptr<CommandExecuter> commander;

  // State -------------------------------------------------------------------
  //
  // These are the types that represent the simulation state and have no
  // functionality of their own, directly. They may change during the
  // simulation.
  //

  /// Track the simulation timing.
  SimulationSync sync;

  /// Track the approximate progress of the simulation.
  SimulationProgress progress;

  /// Non-owning pointer used in order to keep track which model is being
  /// initialized in the CONNECT state in order to allow it to be directly
  /// aborted if it is hanging during initialization. If no model is being
  /// actively initialized the valid value is nullptr.
  cloe::Model* now_initializing{nullptr};

  std::map<std::string, std::unique_ptr<cloe::Simulator>> simulators;
  std::map<std::string, std::shared_ptr<cloe::Vehicle>> vehicles;
  std::map<std::string, std::unique_ptr<cloe::Controller>> controllers;

  timer::DurationTimer<cloe::Duration> cycle_duration;

  /// Tell the simulation that we want to transition into the PAUSE state.
  ///
  /// We can't do this directly via an interrupt because we can only go
  /// into the PAUSE state after STEP_END.
  bool pause_execution{false};

  // Output ------------------------------------------------------------------
  SimulationStatistics statistics;
  std::optional<SimulationOutcome> outcome;
  std::optional<SimulationResult> result;
  std::optional<SimulationProbe> probe;

  // Events ------------------------------------------------------------------
  //
  // The following callbacks store listeners on the given events.
  // In the state where an event occurs, the callback is then triggered.
  // There is generally only one place where each of these callbacks is
  // triggered.
  //
  std::shared_ptr<events::LoopCallback> callback_loop;
  std::shared_ptr<events::PauseCallback> callback_pause;
  std::shared_ptr<events::ResumeCallback> callback_resume;
  std::shared_ptr<events::StartCallback> callback_start;
  std::shared_ptr<events::StopCallback> callback_stop;
  std::shared_ptr<events::SuccessCallback> callback_success;
  std::shared_ptr<events::FailureCallback> callback_failure;
  std::shared_ptr<events::ResetCallback> callback_reset;
  std::shared_ptr<events::TimeCallback> callback_time;

 public:
  // Helper Methods ----------------------------------------------------------
  //
  // These methods encapsulate methods on the data in this struct that can be
  // used by various states. They constitute implementation details and may
  // be refactored out of this struct at some point.
  //
  std::string version() const;
  cloe::Logger logger() const;

  std::shared_ptr<cloe::Registrar> simulation_registrar();

  std::vector<std::string> model_ids() const;
  std::vector<std::string> simulator_ids() const;
  std::vector<std::string> controller_ids() const;
  std::vector<std::string> vehicle_ids() const;
  std::vector<std::string> plugin_ids() const;

  bool foreach_model(std::function<bool(cloe::Model&, const char* type)> f);
  bool foreach_model(std::function<bool(const cloe::Model&, const char* type)> f) const;
  bool foreach_simulator(std::function<bool(cloe::Simulator&)> f);
  bool foreach_simulator(std::function<bool(const cloe::Simulator&)> f) const;
  bool foreach_controller(std::function<bool(cloe::Controller&)> f);
  bool foreach_controller(std::function<bool(const cloe::Controller&)> f) const;
  bool foreach_vehicle(std::function<bool(cloe::Vehicle&)> f);
  bool foreach_vehicle(std::function<bool(const cloe::Vehicle&)> f) const;
};

}  // namespace engine
