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

#include <filesystem>  // for path
#include <functional>  // for function<>
#include <optional>    // for optional<>

#include <sol/state_view.hpp>  // for state_view

#include "stack.hpp"  // for Stack

namespace engine {

class SimulationContext;
class SimulationMachine;
class SimulationResult;
class SimulationProbe;

class Simulation {
 public:
  Simulation(const Simulation&) = default;
  Simulation(Simulation&&) = delete;
  Simulation& operator=(const Simulation&) = default;
  Simulation& operator=(Simulation&&) = delete;
  Simulation(cloe::Stack&& config, sol::state_view lua, const std::string& uuid);
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
   * Probe a simulation.
   *
   * This connects and enrolls, but does not start the simulation.
   */
  SimulationProbe probe();

  /**
   * Write simulation output into files and return number of files written.
   */
  size_t write_output(const SimulationResult&) const;

  /**
   * Write the given JSON output into the file. Return true if successful.
   */
  bool write_output_file(const std::filesystem::path& filepath, const cloe::Json& j) const;

  /**
   * Check if the given filepath may be opened, respecting clobber options.
   */
  bool is_writable(const std::filesystem::path& filepath) const;

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
  /**
   * Determine the output directory from config.
   *
   * Must be called before output_dir is used.
   */
  void set_output_dir();

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
  std::filesystem::path get_output_filepath(const std::filesystem::path& filename) const;

  /**
   * Create the default abort handler that can be used by signal_abort() on
   * this Simulation instance. The return value can be assigned to abort_fn_.
   *
   * It is important that the lifetime of all passed arguments exceeds that
   * of the returned function! Before they are removed, call reset_abort_handler().
   */
  void set_abort_handler(SimulationMachine& machine, SimulationContext& ctx,
                         std::function<void()> hook);

  /**
   * Reset the abort handler before it becomes invalid.
   */
  void reset_abort_handler() { abort_fn_ = nullptr; }

 private:
  cloe::Stack config_;
  sol::state_view lua_;
  cloe::Logger logger_;
  std::string uuid_;
  std::optional<std::filesystem::path> output_dir_;
  std::function<void()> abort_fn_;

  // Options:
  bool report_progress_{false};
};

}  // namespace engine
