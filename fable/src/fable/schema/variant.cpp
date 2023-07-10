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
 * \file fable/schema/variant.cpp
 * \see  fable/schema/variant.hpp
 */

#include <fable/schema/variant.hpp>

#include <cassert>  // for assert
#include <map>      // for map<>
#include <string>   // for string
#include <utility>  // for move, make_pair

#include <fmt/format.h>  // for format

#include <fable/error.hpp>

namespace fable {
namespace schema {

Variant::Variant(std::string desc, std::vector<Box>&& vec)
    : desc_(std::move(desc)), schemas_(std::move(vec)) {
  assert(!schemas_.empty());

  // Calculate type and type string
  std::map<JsonType, std::string> primitive_types;
  std::vector<std::string> variant_types;
  for (const auto& s : schemas_) {
    if (s.is_variant()) {
      variant_types.emplace_back(s.type_string());
    } else {
      primitive_types[s.type()] = s.type_string();
    }
  }

  if (primitive_types.size() == 1 && variant_types.empty()) {
    type_ = primitive_types.begin()->first;
    type_string_ = to_string(type_);
  } else {
    type_ = JsonType::null;
    type_string_ = "union of {";
    unsigned n = 0;
    for (const auto& ts : primitive_types) {
      if (n != 0) {
        type_string_ += ", ";
      }
      type_string_ += ts.second;
      n++;
    }
    for (const auto& s : variant_types) {
      if (n != 0) {
        type_string_ += ", ";
      }
      type_string_ += s;
      n++;
    }
    type_string_ += "}";
  }
}

Json Variant::usage() const {
  auto required = required_ ? "!" : "";
  if (desc_.empty()) {
    return type_string() + required;
  } else {
    return fmt::format("{}{} :: {}", type_string(), required, desc_);
  }
}

Json Variant::json_schema() const {
  Json j;
  if (schemas_.empty()) {
    j["not"] = Json{
        {"description", "no variants available"},
    };
  } else {
    Json types;
    for (const auto& s : schemas_) {
      types.push_back(s.json_schema());
    }
    j[unique_match_ ? "oneOf" : "anyOf"] = std::move(types);
  }
  if (this->has_description()) {
    j["description"] = this->description();
  }
  return j;
}

std::optional<size_t> Variant::validate_index(const Conf& c,
                                              std::optional<SchemaError>& err) const {
  std::vector<size_t> matches;
  std::vector<SchemaError> errors;
  for (size_t i = 0; i < schemas_.size(); ++i) {
    std::optional<SchemaError> tmp;
    if (schemas_[i].validate(c, tmp)) {
      matches.push_back(i);
    } else {
      errors.emplace_back(std::move(*tmp));
    }
  }

  if (matches.size() == 0) {
    err.emplace(SchemaError{c, json_schema(), Json{{"errors", errors}},
                            "input does not match any variants"});
    return {};
  } else if (matches.size() > 1 && unique_match_) {
    err.emplace(SchemaError{c, json_schema(), Json{{"matches", matches}},
                            "input matches more than one variant"});
    return {};
  } else {
    return matches[0];
  }
}

size_t Variant::variant_index(const Conf& c) const {
  std::optional<SchemaError> err;
  auto index = validate_index(c, err);
  if (index) {
    return *index;
  } else {
    throw std::move(*err);
  }
}

void Variant::reset_ptr() {
  for (auto& s : schemas_) {
    s.reset_ptr();
  }
}

}  // namespace schema
}  // namespace fable
