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
 * \file cloe/sync.hpp
 */

#pragma once

#include <cstdint>  // for uint64_t

#include <cloe/core/duration.hpp>  // for Duration, to_convenient_json
#include <fable/json.hpp>          // for Json

namespace cloe {

/**
 * Sync is the synchronization context of the simulation.
 */
class Sync {
 public:
  /**
   * Return the absolute simulation step number.
   *
   * This value is monotonically increasing, and greater-or-equal to zero.
   */
  virtual uint64_t step() const = 0;

  /**
   * Return the atomic simulation step width.
   *
   * This is the lowest-common-denominator of all models.
   */
  virtual Duration step_width() const = 0;

  /**
   * Return the simulation time.
   */
  virtual Duration time() const = 0;

  /**
   * Return the estimated simulation time-of-arrival.
   *
   * This is the time at which the simulation is stopped by a trigger, which
   * could be "stop", "fail", or "restart". If no such ETA is known, zero
   * is returned.
   */
  virtual Duration eta() const = 0;

  /**
   * Return the target simulation factor, with 1.0 being realtime.
   *
   * - If target realtime factor is < 0.0, then it is interpreted to be
   *   unlimited.
   * - If target realtime factor is 0.0, then this is interpreted to be
   *   a paused state.
   * - Currently, the floating INFINITY value is not handled specially.
   */
  virtual double realtime_factor() const = 0;

  /**
   * Return true if there is no target realtime factor, i.e., the simulation
   * runs as fast as possible.
   */
  virtual bool is_realtime_factor_unlimited() const { return realtime_factor() < 0.0; }

  /**
   * Return the maximum theorically achievable simulation realtime factor,
   * with 1.0 being realtime.
   */
  virtual double achievable_realtime_factor() const = 0;

  /**
   * Write the JSON representation into j.
   */
  friend void to_json(fable::Json& j, const Sync& s);
};

/**
 * Write the JSON representation of Sync into j.
 */
inline void to_json(fable::Json& j, const Sync& s) {
  j = fable::Json{
      {"step", s.step()},
      {"step_width", s.step_width()},
      {"time", to_convenient_json(s.time())},
      {"realtime_factor", s.realtime_factor()},
      {"achievable_realtime_factor", s.achievable_realtime_factor()},
  };
  if (s.eta().count() == 0) {
    j["eta"] = nullptr;
  } else {
    j["eta"] = to_convenient_json(s.eta());
  }
}

}  // namespace cloe
