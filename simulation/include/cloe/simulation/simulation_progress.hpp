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
 * \file simulation_progress.hpp
 */

#pragma once

#include <string>
#include <chrono>

#include <cloe/core.hpp>

#include "utility/progress.hpp"    // for Progress

namespace engine {

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

}  // namespace engine
