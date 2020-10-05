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
 * \file simulation.hpp
 * \see  simulation.cpp
 */

#pragma once

#include <functional>  // for function<>
#include <memory>      // for unique_ptr<>

#include <boost/filesystem/path.hpp>  // for path

#include <fable/enum.hpp>  // for ENUM_SERIALIZATION

#include "simulation_context.hpp"
#include "stack.hpp"  // for Stack

namespace engine {

struct SimulationResult {
  cloe::Stack config;

  std::string uuid;
  SimulationSync sync;
  cloe::Duration elapsed;
  SimulationOutcome outcome;
  SimulationStatistics statistics;
  cloe::Json triggers;

 public:
  friend void to_json(cloe::Json& j, const SimulationResult& r) {
    j = cloe::Json{
        {"uuid", r.uuid},       {"statistics", r.statistics}, {"simulation", r.sync},
        {"elapsed", r.elapsed}, {"outcome", r.outcome},
    };
  }
};

class Simulation {
 public:
  Simulation(const cloe::Stack& config, const std::string& uuid);
  ~Simulation() = default;

  /**
   * Return simulation logger.
   */
  cloe::Logger logger() const { return logger_; }

  /**
   * Run a simulation to completion.
   *
   * Requires the entire engine to be configured first.
   * This will throw an exception on failure.
   */
  SimulationResult run();

  /**
   * Write simulation output into files and return number of files written.
   */
  size_t write_output(const SimulationResult&) const;

  /**
   * Write the given JSON output into the file, respecting clobber options.
   * Return true if successful.
   */
  bool write_output_file(const boost::filesystem::path& filepath, const cloe::Json& j) const;

  /**
   * Set whether simulation progress should be reported.
   */
  void set_report_progress(bool value) { report_progress_ = value; }

  /**
   * Abort the simulation from a separate thread.
   *
   * This is used exclusively for handling signals.
   */
  void signal_abort();

 private:
  cloe::Logger logger_;
  cloe::Stack config_;
  std::string uuid_;
  std::function<void()> abort_fn_;

  // Options:
  bool report_progress_{false};
};

}  // namespace engine
