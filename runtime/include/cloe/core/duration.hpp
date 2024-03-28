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
 * \file cloe/core/duration.hpp
 * \see  cloe/core.hpp
 */

#pragma once

#include <chrono>  // for duration<>, duration_cast<>, nanoseconds
#include <ratio>   // for micro, milli
#include <string>  // for string

#include <nlohmann/json.hpp>  // for Json, adl_serializer

namespace cloe {

/**
 * Duration is an alias for std::chrono::nanoseconds, which represents the
 * default unit of time for simulated time and spans roughly +/- 290 years.
 *
 * Warning: the default constructor does not exist. If you want zero time, you
 * need to set it explicitly, for example inside a class:
 * ```cpp
 * Duration time_ = Duration(0);
 * ```
 */
using Duration = std::chrono::nanoseconds;

using Microseconds = std::chrono::duration<double, std::micro>;
using Milliseconds = std::chrono::duration<double, std::milli>;
using Seconds = std::chrono::duration<double>;

std::string to_string(const Duration& ns);

/**
 * Convert a string containing a number and a unit to a duration.
 *
 * The following units are supported:
 *
 *     ns
 *     us
 *     ms
 *     s
 *
 * Will throw an exception on malformed or out-of-range input.
 *
 * Note: This parse function preserves precision even for floating
 * point numbers. For example, 0.1 is not exactly representable
 * as a floating point number, but together with a unit, we can
 * scale it so that it is represented exactly.
 */
Duration parse_duration(const std::string& fmt);

// Occasionally, we want to have a human-readable and a machine-readable
// representation of a duration. This function does exactly that.
nlohmann::json to_convenient_json(const Duration& ns);

}  // namespace cloe

/*
 * In order to provide serialization for third-party types, we need to either
 * use their namespace or provide a specialization in that of nlohmann. It is
 * illegal to define anything in the std namespace, so we are left no choice in
 * this regard.
 *
 * See: https://github.com/nlohmann/json
 */
namespace nlohmann {

inline std::string to_string_hr(double d) {
  auto n = std::to_string(d);
  n.erase(n.find_last_not_of('0') + 1, std::string::npos);
  if (n[n.size() - 1] == '.') {
    n.pop_back();
  }
  return n;
}

template <>
struct adl_serializer<cloe::Microseconds> {
  static void to_json(json& j, const cloe::Microseconds& us) {
    j = to_string_hr(us.count()) + "us";
  }
};

template <>
struct adl_serializer<cloe::Milliseconds> {
  static void to_json(json& j, const cloe::Milliseconds& ms) {
    j = to_string_hr(ms.count()) + "ms";
  }
};

template <>
struct adl_serializer<cloe::Seconds> {
  static void to_json(json& j, const cloe::Seconds& s) { j = to_string_hr(s.count()) + "s"; }
};

template <>
struct adl_serializer<cloe::Duration> {
  static void to_json(json& j, const cloe::Duration& ns) {
    auto count = ns.count();
    if (count > 1e9) {
      j = std::chrono::duration_cast<cloe::Seconds>(ns);
    } else if (count > 1e6) {
      j = std::chrono::duration_cast<cloe::Milliseconds>(ns);
    } else if (count > 1e3) {
      j = std::chrono::duration_cast<cloe::Microseconds>(ns);
    } else {
      j = to_string_hr(count) + "ns";
    }
  }
};

}  // namespace nlohmann
