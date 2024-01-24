#pragma once

#include "cloe/sync.hpp"
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
  static constexpr std::chrono::milliseconds default_step_width{20};
  cloe::Duration step_width_{default_step_width};
};
}
