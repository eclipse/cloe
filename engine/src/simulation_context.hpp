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

#include <cstdint>     // for uint64_t
#include <functional>  // for function<>
#include <map>         // for map<>
#include <memory>      // for unique_ptr<>, shared_ptr<>
#include <string>      // for string
#include <vector>      // for vector<>

#include <boost/optional.hpp>  // for optional<>

#include <cloe/controller.hpp>          // for Controller
#include <cloe/core.hpp>                // for Duration
#include <cloe/registrar.hpp>           // for Registrar
#include <cloe/simulator.hpp>           // for Simulator
#include <cloe/sync.hpp>                // for Sync
#include <cloe/trigger/nil_event.hpp>   // for DEFINE_NIL_EVENT
#include <cloe/utility/statistics.hpp>  // for Accumulator
#include <cloe/utility/timer.hpp>       // for DurationTimer
#include <cloe/vehicle.hpp>             // for Vehicle

#include "coordinator.hpp"         // for Coordinator
#include "registrar.hpp"           // for Registrar
#include "server.hpp"              // for Server
#include "stack.hpp"               // for Stack
#include "utility/command.hpp"     // for CommandExecuter
#include "utility/progress.hpp"    // for Progress
#include "utility/time_event.hpp"  // for TimeCallback

namespace engine {

/**
 * SimulationSync is the synchronization context of the simulation.
 */
class SimulationSync : public cloe::Sync {
 public:  // Overrides
  SimulationSync() = default;
  explicit SimulationSync(const cloe::Duration& step_width) : step_width_(step_width) {}

  uint64_t step() const override { return step_; }
  cloe::Duration step_width() const override { return step_width_; }
  cloe::Duration time() const override { return time_; }
  cloe::Duration eta() const override { return eta_; }

  /**
   * Return the target simulation factor, with 1.0 being realtime.
   *
   * - If target realtime factor is <= 0.0, then it is interpreted to be unlimited.
   * - Currently, the floating INFINITY value is not handled specially.
   */
  double realtime_factor() const override { return realtime_factor_; }

  /**
   * Return the maximum theorically achievable simulation realtime factor,
   * with 1.0 being realtime.
   */
  double achievable_realtime_factor() const override {
    return static_cast<double>(step_width().count()) / static_cast<double>(cycle_time_.count());
  }

 public:  // Modification
  /**
   * Increase the step number for the simulation.
   *
   * - It increases the step by one.
   * - It moves the simulation time forward by the step width.
   * - It stores the real time difference from the last time IncrementStep was called.
   */
  void increment_step() {
    step_ += 1;
    time_ += step_width_;
  }

  /**
   * Set the target realtime factor, with any value less or equal to zero
   * unlimited.
   */
  void set_realtime_factor(double s) { realtime_factor_ = s; }

  void set_eta(cloe::Duration d) { eta_ = d; }

  void reset() {
    time_ = cloe::Duration(0);
    step_ = 0;
  }

  void set_cycle_time(cloe::Duration d) { cycle_time_ = d; }

 private:
  // Simulation State
  uint64_t step_{0};
  cloe::Duration time_{0};
  cloe::Duration eta_{0};
  cloe::Duration cycle_time_;

  // Simulation Configuration
  double realtime_factor_{1.0};            // realtime
  cloe::Duration step_width_{20'000'000};  // should be 20ms
};

/**
 * SimulationProgress represents the progress of the simulation, split into
 * initialization and execution phases.
 */
class SimulationProgress {
  using TimePoint = std::chrono::steady_clock::time_point;

 public:
  std::string stage{""};
  std::string message{"initializing engine"};

  Progress initialization;
  size_t initialization_n;
  size_t initialization_k;

  Progress execution;
  cloe::Duration execution_eta{0};

  // Reporting:
  double report_granularity_p{0.1};
  cloe::Duration report_granularity_d{10'000'000'000};
  double execution_report_p;
  TimePoint execution_report_t;

 public:
  void init_begin(size_t n) {
    message = "initializing";
    initialization.begin();
    initialization_n = n;
    initialization_k = 0;
  }

  void init(const std::string& what) {
    stage = what;
    message = "initializing " + what;
    initialization_k++;
    double p = static_cast<double>(initialization_k) / static_cast<double>(initialization_n);
    initialization.update(p);
  }

  void init_end() {
    initialization_k++;
    assert(initialization_k == initialization_n);
    initialization.end();
    stage = "";
    message = "initialization done";
  }

  bool is_init_ended() const { return initialization.is_ended(); }

  cloe::Duration elapsed() const {
    if (is_init_ended()) {
      return initialization.elapsed() + execution.elapsed();
    } else {
      return initialization.elapsed();
    }
  }

  void exec_begin() {
    stage = "simulation";
    message = "executing simulation";
    execution_report_p = 0;
    execution_report_t = std::chrono::steady_clock::now();
    execution.begin();
  }

  void exec_update(double p) { execution.update_safe(p); }

  void exec_update(cloe::Duration now) {
    if (execution_eta != cloe::Duration(0)) {
      double now_d = static_cast<double>(now.count());
      double eta_d = static_cast<double>(execution_eta.count());
      exec_update(now_d / eta_d);
    }
  }

  void exec_end() {
    stage = "";
    message = "simulation done";
    execution.end();
  }

  bool is_exec_ended() const { return execution.is_ended(); }

  /**
   * Return true and store the current progress percentage and time if the
   * current percentage is granularity_p ahead or at least granularity_d has
   * elapsed since the last report.
   */
  bool exec_report() {
    // We should not report 100% more than once.
    if (execution_report_p == 1.0) {
      return false;
    }

    // If there is no execution ETA, also don't report.
    if (execution_eta == cloe::Duration(0)) {
      return false;
    }

    // Certain minimum percentage has passed.
    auto now = std::chrono::steady_clock::now();
    if (execution.is_ended()) {
      // We should report 100% at least once.
      execution_report_p = 1.0;
      execution_report_t = now;
      return true;
    } else if (execution.percent() - execution_report_p > report_granularity_p) {
      // We should report at least every report_granularity_p (percent).
      execution_report_p = execution.percent();
      execution_report_t = now;
      return true;
    } else if (cast_duration(now - execution_report_t) > report_granularity_d) {
      // We should report at least every report_granularity_d (duration).
      execution_report_p = execution.percent();
      execution_report_t = now;
      return true;
    } else {
      return false;
    }
  }

  friend void to_json(cloe::Json& j, const SimulationProgress& p) {
    j = cloe::Json{
        {"message", p.message},
        {"initialization", p.initialization},
    };
    if (p.execution_eta > cloe::Duration(0)) {
      j["execution"] = p.execution;
    } else {
      j["execution"] = nullptr;
    }
  }
};

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
 * This clearly separates data from functionality.
 */
struct SimulationContext {
  // Setup
  std::unique_ptr<Server> server;
  std::shared_ptr<Coordinator> coordinator;
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
  boost::optional<SimulationOutcome> outcome;
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
