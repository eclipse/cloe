/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file fable/utility/chrono.hpp
 */

#pragma once

#include <chrono>
#include <string>

#include <nlohmann/json.hpp>

namespace fable {

std::chrono::nanoseconds parse_duration_to_nanoseconds(const std::string& s);

/**
 * Convert a string containing a number and a unit to a duration.
 *
 * The following units are supported:
 *
 *  - ns | nanosecond | nanoseconds
 *  - us | microsecond | microseconds
 *  - ms | millisecond | milliseconds
 *  - s | second | seconds
 *  - min | minute | minutes
 *  - h | hour | hours
 *
 * Will throw an exception on malformed or out-of-range input:
 *
 *  - std::invalid_argument if unit missing or unknown
 *  - std::out_of_range if sub-nanosecond precision used (e.g. 0.5ns)
 *
 * Note: This parse function preserves precision even for floating
 * point numbers. For example, 0.1 is not exactly representable
 * as a floating point number, but together with a unit, we can
 * scale it so that it is represented exactly.
 */
template <typename Duration>
Duration parse_duration(const std::string& s) {
  return std::chrono::duration_cast<Duration>(parse_duration_to_nanoseconds(s));
}

template <typename Duration>
std::string to_string(const Duration& d) {
  return to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(d));
}

template <>
std::string to_string(const std::chrono::nanoseconds& d);

}  // namespace fable

namespace nlohmann {

template <typename Rep, typename Period>
struct adl_serializer<std::chrono::duration<Rep, Period>> {
  using Duration = std::chrono::duration<Rep, Period>;

  static void to_json(json& j, const Duration& d) {
    j = ::fable::to_string(d);
  }

  static void from_json(const json& j, Duration& d) {
    std::string s = j.get<std::string>();
    d = ::fable::parse_duration<Duration>(s);
  }
};

}  // namespace nlohmann
