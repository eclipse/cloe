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

#include <cstdint>                      // for uint64_t
#include <functional>                   // for function<>
#include <map>                          // for map<>
#include <memory>                       // for unique_ptr<>, shared_ptr<>
#include <optional>                     // for optional<>
#include <string>                       // for string
#include <vector>                       // for vector<>

#include <sol/state_view.hpp>           // for state_view

#include <cloe/cloe_fwd.hpp>            // for Simulator, Controller, Registrar, Vehicle, Duration
#include <cloe/data_broker.hpp>         // for DataBroker
#include <cloe/sync.hpp>                // for Sync
#include <cloe/trigger/nil_event.hpp>   // for DEFINE_NIL_EVENT
#include <cloe/utility/statistics.hpp>  // for Accumulator
#include <cloe/utility/timer.hpp>       // for DurationTimer
#include <cloe/coordinator.hpp>  // for Coordinator
#include <cloe/simulation_driver.hpp>

#include "registrar.hpp"            // for Registrar
#include "server.hpp"               // for Server
#include "simulation_progress.hpp"  // for SimulationProgress
#include "simulation_sync.hpp"
#include "cloe/stacklib/stack.hpp"               // for Stack
#include "utility/command.hpp"     // for CommandExecuter
#include "utility/time_event.hpp"  // for TimeCallback

namespace engine {



struct SimulationStatistics {
  cloe::utility::Accumulator engine_time_ms;
  cloe::utility::Accumulator cycle_time_ms;
  cloe::utility::Accumulator simulator_time_ms;
  cloe::utility::Accumulator controller_time_ms;
  cloe::utility::Accumulator padding_time_ms;
  cloe::utility::Accumulator controller_retries;

  void reset() {
    engine_time_ms.reset();
    cycle_time_ms.reset();
    simulator_time_ms.reset();
    controller_time_ms.reset();
    padding_time_ms.reset();
    controller_retries.reset();
  }

  friend void to_json(cloe::Json& j, const SimulationStatistics& s) {
    j = cloe::Json{
        {"engine_time_ms", s.engine_time_ms},         {"simulator_time_ms", s.simulator_time_ms},
        {"controller_time_ms", s.controller_time_ms}, {"padding_time_ms", s.padding_time_ms},
        {"cycle_time_ms", s.cycle_time_ms},           {"controller_retries", s.controller_retries},
    };
  }
};

/**
 * SimulationOutcome describes the possible outcomes a simulation can have.
 */
enum class SimulationOutcome {
  NoStart,  ///< Simulation unable to start.
  Aborted,  ///< Simulation aborted due to technical problems or interrupt.
  Stopped,  ///< Simulation concluded, but without valuation.
  Failure,  ///< Simulation explicitly concluded with failure.
  Success,  ///< Simulation explicitly concluded with success.
};

// If possible, the following exit codes should not be used as they are used
// by the Bash shell, among others: 1-2, 126-165, and 255. That leaves us
// primarily with the range 3-125, which should suffice for our purposes.
// The following exit codes should not be considered stable.
#define EXIT_OUTCOME_SUCCESS EXIT_SUCCESS  // normally 0
#define EXIT_OUTCOME_UNKNOWN EXIT_FAILURE  // normally 1
#define EXIT_OUTCOME_NOSTART 4             // 0b.....1..
#define EXIT_OUTCOME_STOPPED 8             // 0b....1...
#define EXIT_OUTCOME_FAILURE 9             // 0b....1..1
#define EXIT_OUTCOME_ABORTED 16            // 0b...1....

// clang-format off
ENUM_SERIALIZATION(SimulationOutcome, ({
    {SimulationOutcome::Aborted, "aborted"},
    {SimulationOutcome::NoStart, "no-start"},
    {SimulationOutcome::Failure, "failure"},
    {SimulationOutcome::Success, "success"},
    {SimulationOutcome::Stopped, "stopped"},
}))
// clang-format on

namespace events {

DEFINE_NIL_EVENT(Start, "start", "start of simulation")
DEFINE_NIL_EVENT(Stop, "stop", "stop of simulation")
DEFINE_NIL_EVENT(Success, "success", "simulation success")
DEFINE_NIL_EVENT(Failure, "failure", "simulation failure")
DEFINE_NIL_EVENT(Reset, "reset", "reset of simulation")
DEFINE_NIL_EVENT(Pause, "pause", "pausation of simulation")
DEFINE_NIL_EVENT(Resume, "resume", "resumption of simulation after pause")
DEFINE_NIL_EVENT(Loop, "loop", "begin of inner simulation loop each cycle")

}  // namespace events

/**
 * SimulationContext represents the entire context of a running simulation.
 *
 * This clearly separates data from functionality. There is no constructor
 * where extra initialization is performed. Instead any initialization is
 * performed in the simulation states in the `simulation.cpp` file.
 */
struct SimulationContext {
  SimulationContext(cloe::SimulationDriver* driver) : simulation_driver(driver) {}

  cloe::SimulationDriver* simulation_driver;

  // Setup
  std::unique_ptr<cloe::DataBroker> db;
  std::unique_ptr<Server> server;
  std::shared_ptr<cloe::coordinator::Coordinator> coordinator;
  std::shared_ptr<Registrar> registrar;
  std::unique_ptr<CommandExecuter> commander;

  // Configuration
  cloe::Stack config;
  std::string uuid{};
  bool report_progress{false};

  // State
  SimulationSync sync;
  SimulationProgress progress;
  SimulationStatistics statistics;
  cloe::Model* now_initializing{nullptr};
  std::map<std::string, std::unique_ptr<cloe::Simulator>> simulators;
  std::map<std::string, std::shared_ptr<cloe::Vehicle>> vehicles;
  std::map<std::string, std::unique_ptr<cloe::Controller>> controllers;
  std::optional<SimulationOutcome> outcome;
  timer::DurationTimer<cloe::Duration> cycle_duration;
  bool pause_execution{false};

  // Events
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
  std::string version() const;
  cloe::Logger logger() const { return cloe::logger::get("cloe"); }

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
