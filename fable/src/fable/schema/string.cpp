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
 * \file fable/schema/string.cpp
 * \see  fable/schema/string.hpp
 */

#include <fable/schema/string.hpp>

#include <algorithm>  // for find
#include <limits>     // for numeric_limits
#include <regex>      // for regex, regex_match
#include <string>     // for string

#include <fable/environment.hpp>  // for interpolate_vars

namespace fable {
namespace schema {

size_t String::min_length() const { return min_length_; }
void String::set_min_length(size_t value) { min_length_ = value; }
String String::min_length(size_t value) && {
  min_length_ = value;
  return std::move(*this);
}

String String::not_empty() && {
  set_min_length(1);
  return std::move(*this);
}

size_t String::max_length() const { return max_length_; }
void String::set_max_length(size_t value) { max_length_ = value; }
String String::max_length(size_t value) && {
  max_length_ = value;
  return std::move(*this);
}

const std::string& String::pattern() const { return pattern_; }
void String::set_pattern(const std::string& value) { pattern_ = value; }
String String::pattern(const std::string& value) && {
  pattern_ = value;
  return std::move(*this);
}

String String::c_identifier() && {
  set_pattern(FABLE_REGEX_C_IDENTIFIER);
  return std::move(*this);
}

bool String::interpolate() const { return interpolate_; }
void String::set_interpolate(bool value) { interpolate_ = value; }
String String::interpolate(bool value) && {
  interpolate_ = value;
  return std::move(*this);
}

Environment* String::environment() const { return env_; }
void String::set_environment(Environment* env) { env_ = env; }
String String::environment(Environment* env) && {
  env_ = env;
  return std::move(*this);
}

const std::vector<std::string>& String::enum_of() const { return enum_; }
void String::set_enum_of(std::vector<std::string>&& init) { enum_ = std::move(init); }
String String::enum_of(std::vector<std::string>&& init) {
  enum_ = std::move(init);
  return std::move(*this);
}

Json String::json_schema() const {
  Json j{
      {"type", "string"},
  };
  if (!pattern_.empty()) {
    j["pattern"] = pattern_;
  }
  if (!enum_.empty()) {
    j["enum"] = enum_;
  }
  if (min_length_ != 0) {
    j["minLength"] = min_length_;
  }
  if (max_length_ != std::numeric_limits<size_t>::max()) {
    j["maxLength"] = max_length_;
  }
  this->augment_schema(j);
  return j;
}

bool String::validate(const Conf& c, std::optional<SchemaError>& err) const {
  if (!this->validate_type(c, err)) {
    return false;
  }

  auto src = c.get<std::string>();
  if (interpolate_) {
    try {
      src = interpolate_vars(src, env_);
    } catch (std::exception& e) {
      return this->set_error(err, c, "error interpolating variables: {}", e.what());
    }
  }
  if (src.size() < min_length_) {
    return this->set_error(err, c, "expect minimum string length of {}, got {}", min_length_, src.size());
  }
  if (src.size() > max_length_) {
    return this->set_error(err, c, "expect maximum string length of {}, got {}", max_length_, src.size());
  }
  if (!pattern_.empty() && !std::regex_match(src, std::regex(pattern_))) {
    return this->set_error(err, c, "expect string to match regex '{}': {}", pattern_, src);
  }
  if (!enum_.empty()) {
    if (std::find(enum_.begin(), enum_.end(), src) == enum_.end()) {
      return this->set_error(err, c, "expect string to match one of {}, got {}", Json{enum_}.dump(), src);
    }
  }

  return true;
}  // namespace schema

void String::reset_ptr() { ptr_ = nullptr; }

Json String::serialize(const String::Type& x) const { return x; }

String::Type String::deserialize(const Conf& c) const {
  auto s = c.get<Type>();
  return (interpolate_ ? interpolate_vars(s, env_) : s);
}

void String::to_json(Json& j) const {
  assert(ptr_ != nullptr);
  j = serialize(*ptr_);
}

void String::from_conf(const Conf& c) {
  assert(ptr_ != nullptr);
  *ptr_ = deserialize(c);
}

}  // namespace schema
}  // namespace fable
