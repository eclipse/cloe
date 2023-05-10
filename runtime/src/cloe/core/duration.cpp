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
 * \file cloe/core/duration.cpp
 * \see  cloe/core/duration.hpp
 */

#include <cloe/core/duration.hpp>

#include <chrono>    // for duration_cast
#include <cmath>     // for pow
#include <optional>  // for optional
#include <string>    // for operator+, string, to_string

namespace cloe {
namespace {

// to_string human readable
std::string to_string_hr(double d) {
  auto n = std::to_string(d);
  n.erase(n.find_last_not_of('0') + 1, std::string::npos);
  if (n[n.size() - 1] == '.') {
    n.pop_back();
  }
  return n;
}

}  // anonymous namespace

Duration parse_duration(const std::string& fmt) {
  // Get whole component of the duration
  size_t idx = 0;
  uint64_t whole = std::stoull(fmt, &idx);
  if (idx >= fmt.size()) {
    throw std::invalid_argument("number requires unit to parse");
  }

  // Get any fraction component of duration.
  // Note the number of digits this number has, so we can efficiently
  // add it the result without losing any precision.
  uint64_t fraction = 0;
  size_t fraction_digits = 0;
  if (fmt[idx] == '.') {
    auto prev = ++idx;
    fraction = std::stoull(fmt.substr(idx), &idx);
    fraction_digits = idx;
    idx += prev;
  }
  if (idx >= fmt.size()) {
    throw std::invalid_argument("number requires unit to parse");
  }

  // Read rest of fmt as unit, skipping whitespace between number
  // and the unit.
  while (fmt[idx] == ' ') {
    idx++;
  }
  std::string unit = fmt.substr(idx);

  Duration result(0);
  int ns_exponent;
  if (unit == "ns") {
    ns_exponent = 0;
    result += std::chrono::nanoseconds(whole);
  } else if (unit == "us") {
    ns_exponent = 3;
    result += std::chrono::microseconds(whole);
  } else if (unit == "ms") {
    ns_exponent = 6;
    result += std::chrono::milliseconds(whole);
  } else if (unit == "s") {
    ns_exponent = 9;
    result += std::chrono::seconds(whole);
  } else {
    throw std::invalid_argument("unit not supported yet: " + unit);
  }

  if (fraction) {
    int fraction_exponent = ns_exponent - fraction_digits;
    if (fraction_exponent < 0) {
      throw std::out_of_range("cannot represent sub-nanosecond precision: " + fmt);
    }
    result += std::chrono::nanoseconds(fraction) * static_cast<size_t>(std::pow(10, fraction_exponent));
  }
  return result;
}

std::string to_string(const Duration& ns) {
  auto count = ns.count();
  if (count > 1e9) {
    return to_string_hr(std::chrono::duration_cast<cloe::Seconds>(ns).count()) + "s";
  } else if (count > 1e6) {
    return to_string_hr(std::chrono::duration_cast<cloe::Milliseconds>(ns).count()) + "ms";
  } else if (count > 1e3) {
    return to_string_hr(std::chrono::duration_cast<cloe::Microseconds>(ns).count()) + "us";
  } else {
    return to_string_hr(count) + "ns";
  }
}

nlohmann::json to_convenient_json(const Duration& ns) {
  return nlohmann::json{
      {"str", ns},
      {"ms", std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(ns).count()},
  };
}

}  // namespace cloe
