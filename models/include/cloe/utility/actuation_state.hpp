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
 * \file cloe/utility/actuation_state.hpp
 * \see  cloe/utility/actuation_state.cpp
 */

#pragma once

#include <string>  // for string

#include <boost/optional.hpp>   // for optional<>
#include <fable/fable_fwd.hpp>  // for Json

#include <cloe/core.hpp>                     // for Duration
#include <cloe/utility/actuation_level.hpp>  // for ActuationLevel
#include <cloe/utility/statistics.hpp>       // for Accumulator

namespace cloe {
namespace utility {

struct ActuationState {
  /// The simulation time that the rest of the struct describes.
  Duration time{0};

  /// The simulation step that the rest of the struct describes.
  uint64_t step{0};

  /// Whether AEB is currently triggered.
  bool aeb_active{false};

  /// The official state of control that the actuator should have.
  ActuationLevel fct_control{ActuationLevel::None};

  /// The desired set speed of the controller [m/s].
  boost::optional<double> fct_set_speed;

  /// The desired time gap of the controller [s].
  boost::optional<double> fct_time_gap;

  /// The desired speed limiter velocity of the controller [m/s].
  boost::optional<double> fct_speed_limiter_velocity;

  /// The desired speed limiter velocity of the controller [m/s].
  int fct_speed_limiter_state;

  /// The acceleration request of the controller in [m/s^2].
  boost::optional<double> acceleration;

  /// The steering angle request of the controller in [rad].
  boost::optional<double> steering_angle;

  /// The steering torque request of the controller in [Nm].
  boost::optional<double> steering_torque;

  /// Returns true if the AEB is reported active.
  bool is_aeb_active() const { return aeb_active; }

  /// Returns true if the controller reports to have control.
  bool is_fct_active() const;

  /// Returns true if either AEB or FCT is reported active.
  bool is_active() const { return is_aeb_active() || is_fct_active(); }

  /**
   * Returns true if the controller state is consistent with the level of control it reports.
   *
   * - When control is NONE or STANDBY, then no actuation should occur.
   * - steering_torque and steering_angle should not be set at the same time.
   * - longitudinal and/or lateral movement only if control mode allows it
   * - AEB functionality is always allowed, but only allows deceleration.
   */
  bool is_consistent() const;

  friend void to_json(fable::Json& j, const ActuationState& s);
};

struct ActuationStatistics {
  Accumulator active;
  Accumulator consistent;
  Accumulator aeb_active;
  Pie<ActuationLevel::Enum> fct_control;
  Accumulator fct_active;
  Accumulator fct_set_speed;
  Accumulator fct_time_gap;
  Accumulator acceleration;
  Accumulator steering_angle;
  Accumulator steering_torque;

  void push_back(const ActuationState& s);
  void reset();

  friend void to_json(fable::Json& j, const ActuationStatistics& s);
};

}  // namespace utility
}  // namespace cloe
