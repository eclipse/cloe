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
 * \file fable/schema/string.hpp
 * \see  fable/schema/string.cpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_STRING_HPP_
#define FABLE_SCHEMA_STRING_HPP_

#include <limits>   // for numeric_limits<>
#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {

// Forward declarations:
class Environment;  // from <fable/environment.hpp>

namespace schema {

#define FABLE_REGEX_C_IDENTIFIER "^[a-zA-Z_][a-zA-Z0-9_]*$"

class String : public Base<String> {
 public:  // Types and Constructors
  using Type = std::string;

  String(Type* ptr, std::string&& desc) : Base(JsonType::string, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  String not_empty() && {
    set_min_length(1);
    return std::move(*this);
  }

  String c_identifier() && {
    set_pattern(FABLE_REGEX_C_IDENTIFIER);
    return std::move(*this);
  }

  size_t min_length() const { return min_length_; }
  void set_min_length(size_t value) { min_length_ = value; }
  String min_length(size_t value) && {
    min_length_ = value;
    return std::move(*this);
  }

  size_t max_length() const { return max_length_; }
  void set_max_length(size_t value) { max_length_ = value; }
  String max_length(size_t value) && {
    max_length_ = value;
    return std::move(*this);
  }

  const std::string& pattern() const { return pattern_; }
  void set_pattern(const std::string& value) { pattern_ = value; }
  String pattern(const std::string& value) && {
    pattern_ = value;
    return std::move(*this);
  }

  bool interpolate() const { return interpolate_; }
  void set_interpolate(bool value) { interpolate_ = value; }
  String interpolate(bool value) && {
    interpolate_ = value;
    return std::move(*this);
  }

  Environment* environment() const { return env_; }
  void set_environment(Environment* env) { env_ = env; }
  String environment(Environment* env) && {
    env_ = env;
    return std::move(*this);
  }

 public:  // Overrides
  Json json_schema() const override;
  void validate(const Conf& c) const override;

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = deserialize(c);
  }

  Json serialize(const Type& x) const { return x; }

  Type deserialize(const Conf& c) const;

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  bool interpolate_{false};
  size_t min_length_{0};
  size_t max_length_{std::numeric_limits<size_t>::max()};
  std::string pattern_{};
  Environment* env_{nullptr};
  Type* ptr_{nullptr};
};

inline String make_schema(std::string* ptr, std::string&& desc) {
  return String(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_STRING_HPP_
