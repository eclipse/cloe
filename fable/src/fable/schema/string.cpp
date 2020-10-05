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

#include <limits>  // for numeric_limits
#include <regex>   // for regex, regex_match
#include <string>  // for string

#include <fable/environment.hpp>  // for interpolate_vars

namespace fable {
namespace schema {

Json String::json_schema() const {
  Json j{
      {"type", "string"},
  };
  if (!pattern_.empty()) {
    j["pattern"] = pattern_;
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

void String::validate(const Conf& c) const {
  this->validate_type(c);

  auto src = c.get<std::string>();
  if (interpolate_) {
    src = interpolate_vars(src, env_);
  }
  if (src.size() < min_length_) {
    this->throw_error(c, "expect minimum string length of {}, got {}", min_length_, src.size());
  }
  if (src.size() > max_length_) {
    this->throw_error(c, "expect maximum string length of {}, got {}", max_length_, src.size());
  }
  if (!pattern_.empty() && !std::regex_match(src, std::regex(pattern_))) {
    this->throw_error(c, "expect string to match regex '{}': {}", pattern_, src);
  }
}  // namespace schema

String::Type String::deserialize(const Conf& c) const {
  auto s = c.get<Type>();
  return (interpolate_ ? interpolate_vars(s, env_) : s);
}

}  // namespace schema
}  // namespace fable
