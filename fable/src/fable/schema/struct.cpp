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
 * \file fable/schema/struct.cpp
 * \see  fable/schema/struct.hpp
 */

#include <fable/schema.hpp>
#include <fable/schema/struct.hpp>

#include <algorithm>  // for find
#include <string>     // for string
#include <utility>    // for move, make_pair

namespace fable {
namespace schema {

void Struct::set_property(const std::string& key, Box&& s) {
  auto it = std::find(properties_required_.begin(), properties_required_.end(), key);
  if (s.is_required()) {
    if (it == properties_required_.end()) {
      properties_required_.push_back(key);
    }
  } else {
    if (it != properties_required_.end()) {
      properties_required_.erase(it);
    }
  }
  properties_.insert(std::make_pair(key, std::move(s)));
}

void Struct::set_properties(PropertyList<Box> props) {
  for (auto& p : props) {
    set_property(p.first, p.second.clone());
  }
}

void Struct::set_properties(PropertyList<Schema> props) {
  for (auto& p : props) {
    set_property(p.first, p.second.clone());
  }
}

void Struct::set_properties(const std::map<std::string, Box>& props) {
  for (auto& p : props) {
    set_property(p.first, p.second.clone());
  }
}

void Struct::set_require(std::initializer_list<std::string> init) {
  for (auto& p : init) {
    if (std::find(properties_required_.cbegin(), properties_required_.cend(), p) ==
        properties_required_.cend()) {
      properties_required_.push_back(p);
    }
  }
}

Struct Struct::require(std::initializer_list<std::string> init) && {
  set_require(init);
  return std::move(*this);
}

void Struct::set_require_all() {
  properties_required_.clear();
  for (auto& kv : properties_) {
    properties_required_.push_back(kv.first);
  }
}

Struct Struct::require_all() && {
  set_require_all();
  return std::move(*this);
}

Json Struct::usage() const {
  Json j;
  for (const auto& it : properties_) {
    j[it.first] = it.second.usage();
  }
  return j;
}

Json Struct::json_schema() const {
  Json props;
  for (const auto& kv : properties_) {
    props[kv.first] = kv.second.json_schema();
  }
  Json j{
      {"type", "object"},
      {"properties", props},
      {"additionalProperties", additional_properties_},
  };
  if (additional_prototype_ != nullptr) {
    j["additionalProperties"] = additional_prototype_->json_schema();
  }
  if (!properties_required_.empty()) {
    j["required"] = properties_required_;
  }
  this->augment_schema(j);
  return j;
}

void Struct::validate(const Conf& c) const {
  try {
    this->validate_type(c);
    for (const auto& k : properties_required_) {
      c.assert_has(k);
    }
    for (const auto& kv : c->items()) {
      auto key = kv.key();
      if (properties_.count(key)) {
        properties_.at(key).validate(c.at(key));
      } else if (additional_prototype_ != nullptr) {
        additional_prototype_->validate(c.at(key));
      } else if (additional_properties_) {
        continue;
      } else {
        throw SchemaError{error::UnexpectedProperty(c, key), json_schema()};
      }
    }
  } catch (SchemaError& e) {
    throw;
  } catch (ConfError& e) {
    throw SchemaError{e, json_schema()};
  }
}

void Struct::from_conf(const Conf& c) {
  try {
    for (const auto& k : properties_required_) {
      c.assert_has(k);
    }
    for (const auto& kv : c->items()) {
      auto key = kv.key();
      if (properties_.count(key)) {
        properties_[key].from_conf(c.at(key));
      } else if (additional_prototype_ != nullptr) {
        additional_prototype_->validate(c.at(key));
      } else if (additional_properties_) {
        continue;
      } else {
        throw SchemaError{error::UnexpectedProperty(c, key), json_schema()};
      }
    }
  } catch (SchemaError& e) {
    throw;
  } catch (ConfError& e) {
    throw SchemaError{e, json_schema()};
  }
}

void Struct::to_json(Json& j) const {
  j = Json::object();
  for (const auto& kv : properties_) {
    Json v;
    kv.second.to_json(v);
    if (!v.is_null() || kv.second.is_required()) {
      j[kv.first] = v;
    }
  }
}

void Struct::reset_ptr() {
  for (auto& kv : properties_) {
    kv.second.reset_ptr();
  }
}

}  // namespace schema
}  // namespace fable
