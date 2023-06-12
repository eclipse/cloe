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

#include <fable/utility/chrono.hpp>

#include <chrono>    // for duration_cast
#include <cmath>     // for pow
#include <optional>  // for optional
#include <string>    // for operator+, string, to_string

namespace fable {
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

enum class DurationUnit { Nanosecond, Microsecond, Millisecond, Second, Minute, Hour };

static std::map<std::string, DurationUnit> DURATION_UNITS{
    {"ns", DurationUnit::Nanosecond},
    {"nanosecond", DurationUnit::Nanosecond},
    {"nanoseconds", DurationUnit::Nanosecond},

    {"us", DurationUnit::Microsecond},
    {"microsecond", DurationUnit::Microsecond},
    {"microseconds", DurationUnit::Microsecond},

    {"ms", DurationUnit::Millisecond},
    {"millisecond", DurationUnit::Millisecond},
    {"milliseconds", DurationUnit::Millisecond},

    {"s", DurationUnit::Second},
    {"second", DurationUnit::Second},
    {"seconds", DurationUnit::Second},

    {"s", DurationUnit::Second},
    {"second", DurationUnit::Second},
    {"seconds", DurationUnit::Second},

    {"min", DurationUnit::Minute},
    {"minute", DurationUnit::Minute},
    {"minutes", DurationUnit::Minute},

    {"h", DurationUnit::Hour},
    {"hour", DurationUnit::Hour},
    {"hours", DurationUnit::Hour},
};

/**
 * Parse a duration unit in an efficient way.
 *
 * This accepts the following unit specifications, where the 's' indicating
 * the plural is optional.
 *
 *  ns | nanoseconds?
 *  us | microseconds?
 *  ms | milliseconds?
 *  s | seconds?
 *  min | minutes?
 *  h | | hours?
 */
DurationUnit parse_duration_unit(const std::string& sv) { return DURATION_UNITS.at(sv); }

/**
 * Converts d to a (possibly) fractional string without precision loss.
 *
 * Note: The implementation does not use floating point numbers to avoid
 * any rounding errors that might otherwise occur.
 */
template <typename Unit, typename Duration>
std::string to_string_with_unit(Duration d, std::string_view suffix) {
  std::string buf;
  buf.reserve(16); // This should be enough for most numbers we deal with.

  // Write the whole component (e.g. "1" from "1.5s")
  Unit whole = std::chrono::duration_cast<Unit>(d);
  buf += std::to_string(whole.count());

  // Write fraction, if non-zero (e.g. ".5" from "1.5s"):
  Duration fraction = d - whole;
  if (fraction.count() != 0) {
    buf += ".";

    auto fraction_s = std::to_string(fraction.count());

    // If fraction_s has less digits than the maximum the fraction can have,
    // then we need to left-pad it with zeros.
    auto max_digits = std::log10(Duration::period::den / Unit::period::den);
    buf += std::string("0", max_digits - fraction_s.size());

    // Add the rest of the fraction, but chop off trailing zeros.
    buf += fraction_s.substr(0, fraction_s.find_last_not_of("0")+1);
  }

  // Write suffix, if available (e.g. "s" from "1.5s")
  if (suffix.data()) {
    buf += suffix;
  }
  return buf;
}

}  // anonymous namespace

std::chrono::nanoseconds parse_duration_to_nanoseconds(const std::string& sv) {
  // Get whole component of the duration
  size_t idx = 0;
  uint64_t whole = std::stoull(sv, &idx);
  if (idx >= sv.size()) {
    throw std::invalid_argument("number requires unit to parse");
  }

  // Get any fraction component of duration.
  // Note the number of digits this number has, so we can efficiently
  // add it the result without losing any precision.
  uint64_t fraction = 0;
  size_t fraction_digits = 0;
  if (sv[idx] == '.') {
    auto prev = ++idx;
    fraction = std::stoull(sv.substr(idx), &idx);
    fraction_digits = idx;
    idx += prev;
  }
  if (idx >= sv.size()) {
    throw std::invalid_argument("number requires unit to parse");
  }

  // Read rest of sv as unit, skipping whitespace between number
  // and the unit.
  while (sv[idx] == ' ') {
    idx++;
  }
  DurationUnit unit;
  try {
    unit = parse_duration_unit(sv.substr(idx));
  } catch (std::out_of_range&) {
    throw std::invalid_argument("number requires valid unit to parse");
  }

  std::chrono::nanoseconds result(0);
  int ns_exponent;
  int multiplier = 1;
  switch (unit) {
    case DurationUnit::Nanosecond:
      ns_exponent = 0;
      result += std::chrono::nanoseconds(whole);
      break;
    case DurationUnit::Microsecond:
      ns_exponent = 3;
      result += std::chrono::microseconds(whole);
      break;
    case DurationUnit::Millisecond:
      ns_exponent = 6;
      result += std::chrono::milliseconds(whole);
      break;
    case DurationUnit::Second:
      ns_exponent = 9;
      result += std::chrono::seconds(whole);
      break;
    case DurationUnit::Minute:
      ns_exponent = 10;
      multiplier = 6;
      result += std::chrono::minutes(whole);
      break;
    case DurationUnit::Hour:
      ns_exponent = 11;
      multiplier = 36;
      result += std::chrono::hours(whole);
      break;
  }

  if (fraction) {
    int fraction_exponent = ns_exponent - fraction_digits;
    if (fraction_exponent < 0) {
      throw std::out_of_range("cannot represent sub-nanosecond precision: " + sv);
    }
    result += std::chrono::nanoseconds(fraction * multiplier) *
              static_cast<size_t>(std::pow(10, fraction_exponent));
  }
  return result;
}

template <>
std::string to_string<std::chrono::nanoseconds>(const std::chrono::nanoseconds& ns) {
  auto count = ns.count();
  if (count >= 1e9) {
    return to_string_with_unit<std::chrono::seconds>(ns, "s");
  } else if (count >= 1e6) {
    return to_string_with_unit<std::chrono::milliseconds>(ns, "ms");
  } else if (count >= 1e3) {
    return to_string_with_unit<std::chrono::microseconds>(ns, "us");
  } else {
    return std::to_string(count) + "ns";
  }
}

}  // namespace fable
