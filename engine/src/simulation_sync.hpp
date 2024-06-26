/*
 * Copyright 2024 Robert Bosch GmbH
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
 * \file simulation_sync.hpp
 */

#pragma once

#include <cstdint>

#include <cloe/core/duration.hpp>
#include <cloe/sync.hpp>

namespace engine {

/**
 * SimulationSync is the synchronization context of the simulation.
 */
class SimulationSync : public cloe::Sync {
 public:  // Overrides
  SimulationSync() = default;
  SimulationSync(const SimulationSync &) = default;
  SimulationSync(SimulationSync &&) = delete;
  SimulationSync &operator=(const SimulationSync &) = default;
  SimulationSync &operator=(SimulationSync &&) = delete;
  virtual ~SimulationSync() = default;

  explicit SimulationSync(const cloe::Duration &step_width) : step_width_(step_width) {}

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
  cloe::Duration cycle_time_{0};

  // Simulation Configuration
  double realtime_factor_{1.0};            // realtime
  cloe::Duration step_width_{20'000'000};  // should be 20ms
};

}  // namespace engine
