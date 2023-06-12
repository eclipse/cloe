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
 * \file cloe/utility/actuation_state.cpp
 * \see  cloe/utility/actuation_state.hpp
 */

#include <cloe/utility/actuation_state.hpp>

#include <string>

#include <fable/utility/boost_optional.hpp>

#include <cloe/core.hpp>

namespace cloe {
namespace utility {

bool ActuationState::is_fct_active() const { return fct_control.has_control(); }

bool ActuationState::is_consistent() const {
  assert(fct_control.is_valid());
  bool aeb = aeb_active && acceleration && *acceleration < 0.0;
  switch (fct_control.get_raw()) {
    case ActuationLevel::None:
    // fallthrough, because none looks the same as standby
    case ActuationLevel::Standby:
      return aeb || !(acceleration || steering_angle || steering_torque);
    case ActuationLevel::Long:
      return aeb || (acceleration && !steering_angle && !steering_torque);
    case ActuationLevel::Lat:
      return aeb || (!acceleration && (steering_angle != steering_torque));
    case ActuationLevel::LatLong:
      return aeb || (acceleration && (steering_angle != steering_torque));
    default:
      // Assuming that AEB is active even when the function is not.
      return aeb;
  }
}

void ActuationStatistics::push_back(const ActuationState& s) {
  active.push_back(s.is_active() ? 1.0 : 0.0);
  consistent.push_back(s.is_consistent() ? 1.0 : 0.0);
  aeb_active.push_back(s.is_aeb_active() ? 1.0 : 0.0);
  fct_active.push_back(s.is_fct_active() ? 1.0 : 0.0);
  fct_control.push_back(s.fct_control.get_raw());

#define SET_IFSET(name)        \
  {                            \
    if (s.name) {              \
      name.push_back(*s.name); \
    }                          \
  }

  SET_IFSET(fct_set_speed);
  SET_IFSET(fct_time_gap);
  SET_IFSET(acceleration);
  SET_IFSET(steering_angle);
  SET_IFSET(steering_torque);

#undef SET_IFSET
}

void ActuationStatistics::reset() {
  active.reset();
  consistent.reset();
  aeb_active.reset();
  fct_control.reset();
  fct_active.reset();
  fct_set_speed.reset();
  fct_time_gap.reset();
  acceleration.reset();
  steering_angle.reset();
  steering_torque.reset();
}

// JSON =========================================================================================

void to_json(Json& j, const ActuationState& s) {
  j = Json{
      {"time", s.time},
      {"step", s.step},
      {"any_active", s.is_active()},
      {"consistent", s.is_consistent()},
      {"aeb_active", s.is_aeb_active()},
      {"fct_active", s.is_fct_active()},
      {"fct_control", s.fct_control},
      {"fct_set_speed", s.fct_set_speed},
      {"fct_time_gap", s.fct_time_gap},
      {"acceleration", s.acceleration},
      {"steering_angle", s.steering_angle},
      {"steering_torque", s.steering_torque},
  };

#define SET_JSON(name)    \
  {                       \
    if (s.name) {         \
      j[#name] = *s.name; \
    }                     \
  }

  SET_JSON(fct_set_speed);
  SET_JSON(fct_time_gap);
  SET_JSON(acceleration);
  SET_JSON(steering_angle);
  SET_JSON(steering_torque);

#undef SET_JSON
}

void to_json(Json& j, const ActuationStatistics& s) {
  j = Json{
      {"active", s.active},
      {"consistent", s.consistent},
      {"aeb_active", s.aeb_active},
      {"fct_active", s.fct_active},
      {"fct_control", s.fct_control},
      {"fct_set_speed", s.fct_set_speed},
      {"fct_time_gap", s.fct_time_gap},
      {"acceleration", s.acceleration},
      {"steering_angle", s.steering_angle},
      {"steering_torque", s.steering_torque},
  };
}

}  // namespace utility
}  // namespace cloe
