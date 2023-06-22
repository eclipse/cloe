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
 * \file fable/environment.hpp
 * \see  fable/environment.cpp
 * \see  fable/environment_test.cpp
 */

#pragma once

#include <cassert>  // for assert
#include <map>      // for map<>
#include <string>   // for string
#include <utility>  // for pair<>, move, make_pair
#include <optional> // for optional<>

#include <fable/fable_fwd.hpp>

namespace fable {

class Environment {
 public:
  Environment() = default;
  Environment(std::initializer_list<std::pair<std::string const, std::string>> init)
      : defines_(init) {}
  Environment(const std::map<std::string, std::string>& defines) : defines_(defines) {}
  Environment(std::map<std::string, std::string>&& defines) : defines_(std::move(defines)) {}
  ~Environment() = default;

  bool prefer_external() const { return prefer_external_; }
  void prefer_external(bool value) { prefer_external_ = value; }

  bool allow_undefined() const { return allow_undefined_; }
  void allow_undefined(bool value) { allow_undefined_ = value; }

  void insert(const std::string& key, const std::string& value) {
    assert(defines_.count(key) == 0);
    defines_.insert(std::make_pair(key, value));
  }

  void set(const std::string& key, const std::string& value) { defines_[key] = value; }

  /**
   * Return the value of a literal key, trying both environment and internal
   * defines, depending on the value of perfer_external().
   *
   * If neither are defined, "" is returned when allow_undefined() is true,
   * and a std::out_of_range error is thrown.
   *
   * This is roughly equivalent to ${KEY}.
   */
  std::optional<std::string> get(const std::string& key) const {
    return get(key, prefer_external_);
  }
  std::optional<std::string> get(const std::string& key, bool prefer_external) const;

  /**
   * Return the value of a literal key, trying both environment and internal
   * defines, depending on the value of perfer_external().
   *
   * If neither are defined, alternative is returned.
   *
   * This is equivalent to ${KEY-ALTERNATIVE}, and cannot fail.
   */
  std::string get_or(const std::string& key, const std::string& alternative) const {
    return get(key).value_or(alternative);
  }
  std::string get_or(const std::string& key, const std::string& alternative,
                     bool prefer_external) const {
    return get(key, prefer_external).value_or(alternative);
  }

  /**
   * Return the value of a literal key, trying both environment and internal
   * defines, depending on the value of perfer_external().
   *
   * If neither are defined, a std::out_of_range error is thrown.
   *
   * This is roughly equivalent to ${KEY?out_of_range}.
   */
  std::string require(const std::string& key) const { return require(key, prefer_external_); }
  std::string require(const std::string& key, bool prefer_external) const;

  /**
   * Evaluate a single variable, such as "KEY" or "KEY-ALTERNATIVE".
   *
   * Throws std::out_of_range if allow_undefined() is false and
   * std::invalid_argument if a malformed string is supplied.
   */
  std::string evaluate(const std::string& s) const {
    return evaluate(s, prefer_external_, allow_undefined_);
  }
  std::string evaluate(const std::string& s, bool prefer_external, bool allow_undefined) const;

  /**
   * Interpolate a string will evaluate all variable instances in a string.
   *
   * Throws std::out_of_range if allow_undefined() is false and
   * std::invalid_argument if a malformed string is supplied.
   */
  std::string interpolate(const std::string& s) const {
    return interpolate(s, prefer_external_, allow_undefined_);
  }
  std::string interpolate(const std::string& s, bool prefer_external, bool allow_undefined) const;

 private:
  bool prefer_external_{true};
  bool allow_undefined_{false};
  std::map<std::string, std::string> defines_;
};

inline std::string interpolate_vars(const std::string& s, const Environment* env = nullptr) {
  if (env != nullptr) {
    return env->interpolate(s);
  } else {
    return Environment().interpolate(s);
  }
}

}  // namespace fable
