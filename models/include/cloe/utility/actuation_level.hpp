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
 * \file cloe/utility/actuation_level.hpp
 * \see  cloe/utility/actuation_level_test.cpp
 *
 * This file defines the enum ActuationLevel, which is used to track
 * and visualize the current vehicle control.
 *
 * Currently, the visualization is optimized to be used as a vehicle label
 * with a normal sans font.
 *
 *  *     NONE      Internal control (e.g. VTD)
 *  / \   LONG      External longitudinal control (e.g. ACC)
 *  <->   LAT       External lateral control (e.g. Lane-Assist)
 *  /+\   LONG+LAT  External longitudinal and lateral control (e.g. Highway-Pilot)
 *  ...   STANDBY   Standby state (i.e. controller could be active, but isn't)
 *  !     UNKNOWN   Unknown state (i.e. state has never changed)
 *
 * However, because these symbols could be confusing, we default to a more
 * verbose method of using words.
 */

#pragma once
#ifndef CLOE_UTILITY_ACTUATION_LEVEL_HPP_
#define CLOE_UTILITY_ACTUATION_LEVEL_HPP_

#include <string>  // for string

#include <fable/json.hpp>  // for Json

namespace cloe {
namespace utility {

class ActuationLevel {
 public:
  enum Enum {
    None = 0,
    Long = 1,
    Lat = 2,
    LatLong = 3,
    Standby = 4,
  };

  ActuationLevel() : enum_(None) {}
  ActuationLevel(Enum e) : enum_(e) {}  // NOLINT
  ActuationLevel(bool lat, bool lng) : enum_(None) {
    if (lat) {
      this->set_lat();
    }
    if (lng) {
      this->set_long();
    }
  }

  Enum get_raw() const { return enum_; }
  void set_raw(Enum e) {
    enum_ = e;
    assert(this->is_valid());
  }

  bool is_valid() const {
    // This looks complex, but it isn't that much.
    // Currently, the following states are allowed:
    //
    //  0b100  0b011  0b010  0b001
    //    4     3      2      1
    //
    // In particular, any other combination and any setting of higher bits
    // would push the value of the enum to be greater than 4.
    return enum_ <= 4;
  }

  bool has_lat() const { return (enum_ & Lat) != 0; }
  bool has_long() const { return (enum_ & Long) != 0; }
  bool has_both() const { return enum_ == LatLong; }
  bool has_control() const { return (enum_ & LatLong) != 0; }
  bool is_standby() const { return enum_ == Standby; }
  bool is_none() const { return enum_ == None; }

  void set_none() { enum_ = None; }
  void set_standby() { enum_ = Standby; }
  void set_latlong() { enum_ = LatLong; }
  void set_lat() { enum_ = static_cast<Enum>((enum_ & Long) | Lat); }
  void set_long() { enum_ = static_cast<Enum>((enum_ & Lat) | Long); }

  bool operator==(const ActuationLevel& other) const { return enum_ == other.enum_; }
  bool operator!=(const ActuationLevel& other) const { return !operator==(other); }

  const char* to_human_cstr() const {
    switch (enum_) {
      case None:
        return "none";
      case Long:
        return "longitudinal";
      case Lat:
        return "lateral";
      case LatLong:
        return "longitudinal and lateral";
      case Standby:
        return "standby";
      default:
        return "unknown";
    }
  }

  const char* to_symbol_cstr() const {
    switch (enum_) {
      case None:
        return "*";
      case Long:
        return "/ \\";
      case Lat:
        return "<->";
      case LatLong:
        return "/+\\";
      case Standby:
        return "...";
      default:
        return "!";
    }
  }

  const char* to_loud_cstr() const {
    switch (enum_) {
      case None:
        return "N/A";
      case Long:
        return "LONG";
      case Lat:
        return "LAT";
      case LatLong:
        return "LONG+LAT";
      case Standby:
        return "STANDBY";
      default:
        return "ERROR";
    }
  }

  const char* to_unicode_cstr() const {
    switch (enum_) {
      case None:
        return "✖";
      case Long:
        return "∥";
      case Lat:
        return "≈";
      case LatLong:
        return "∆";
      case Standby:
        return "♤";
      default:
        return "↯";
    }
  }

  const char* to_cstr() const { return this->to_human_cstr(); }

  friend void to_json(fable::Json& j, const ActuationLevel& l) { j = l.to_cstr(); }

 private:
  Enum enum_;
};

// Required for cloe::utility::Pie to work.
inline std::string to_string(const ActuationLevel::Enum level) {
  return std::string(ActuationLevel(level).to_cstr());
}

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_ACTUATION_LEVEL_HPP_
