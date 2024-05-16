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
#include <sol/state.hpp>   // for state

#include "simulation_context.hpp"
#include "stack.hpp"  // for Stack

namespace engine {

struct SimulationResult {
  cloe::Stack config;

  std::string uuid;
  SimulationSync sync;
  cloe::Duration elapsed;
  SimulationOutcome outcome;
  std::vector<std::string> errors;
  SimulationStatistics statistics;
  cloe::Json triggers;
  cloe::Json report;
  cloe::Json signals;  // dump of all signals in DataBroker right before the simulation started
  std::vector<std::string>
      signals_autocompletion;  // pseudo lua file used for vscode autocompletion
  boost::optional<boost::filesystem::path> output_dir;

 public:
  /**
   * The output directory of files is normally built up with:
   *
   *     $registry / $id / $filename
   *
   * If any of the last variables is absolute, the preceding variables
   * shall be ignored; e.g. if $filename is absolute, then neither the
   * simulation registry nor the UUID-based path shall be considered.
   *
   * If not explicitly specified in the configuration file, the registry
   * and output path are set automatically. Thus, if they are empty, then
   * that is because the user explicitly set them so.
   */
  boost::filesystem::path get_output_filepath(const boost::filesystem::path& filename) const {
    boost::filesystem::path filepath;
    if (filename.is_absolute()) {
      filepath = filename;
    } else if (output_dir) {
      filepath = *output_dir / filename;
    } else {
      throw cloe::ModelError{"cannot determine output path for '{}'", filename.native()};
    }

    return filepath;
  }

  /**
   * Determine the output directory from config.
   *
   * Must be called before output_dir is used.
   */
  void set_output_dir() {
    if (config.engine.output_path) {
      // For $registry to be of value, output_path (~= $id) here needs to be set.
      if (config.engine.output_path->is_absolute()) {
        // If it's absolute, then registry_path doesn't matter.
        output_dir = *config.engine.output_path;
      } else if (config.engine.registry_path) {
        // Now, since output_dir is relative, we need the registry path.
        // We don't care here whether the registry is relative or not.
        output_dir = *config.engine.registry_path / *config.engine.output_path;
      }
    }
  }

  friend void to_json(cloe::Json& j, const SimulationResult& r) {
    j = cloe::Json{
        {"elapsed", r.elapsed},
        {"errors", r.errors},
        {"outcome", r.outcome},
        {"report", r.report},
        {"simulation", r.sync},
        {"statistics", r.statistics},
        {"uuid", r.uuid},
    };
  }
};

class Simulation {
 public:
  Simulation(cloe::Stack&& config, sol::state&& lua, const std::string& uuid);
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
   * Write the given JSON output into the file. Return true if successful.
   */
  bool write_output_file(const boost::filesystem::path& filepath, const cloe::Json& j) const;

  /**
   * Check if the given filepath may be opened, respecting clobber options.
   */
  bool is_writable(const boost::filesystem::path& filepath) const;

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
  cloe::Stack config_;
  sol::state lua_;
  cloe::Logger logger_;
  std::string uuid_;
  std::function<void()> abort_fn_;

  // Options:
  bool report_progress_{false};
};

}  // namespace engine
