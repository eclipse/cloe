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
 * \file fable/schema/map.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_MAP_HPP_
#define FABLE_SCHEMA_MAP_HPP_

#include <limits>   // for numeric_limits<>
#include <map>      // for map<>
#include <memory>   // for shared_ptr<>
#include <regex>    // for regex, regex_match
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <boost/optional.hpp>  // for optional<>

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

/**
 * Map maintains a key-value mapping where the list of keys is unknown and
 * the value is always the same type.
 *
 * This should not be confused with the Struct type.
 *
 * \see  fable/schema/struct.hpp
 */
template <typename T, typename P>
class Map : public Base<Map<T, P>> {
 public:  // Types and Constructors
  using Type = std::map<std::string, T>;
  using PrototypeSchema = P;

  Map(Type* ptr, std::string&& desc);
  Map(Type* ptr, const PrototypeSchema& prototype)
      : Base<Map<T, P>>(JsonType::object), prototype_(prototype), ptr_(ptr) {
    prototype_.reset_ptr();
  }
  Map(Type* ptr, const PrototypeSchema& prototype, std::string&& desc)
      : Base<Map<T, P>>(JsonType::object, std::move(desc)), prototype_(prototype), ptr_(ptr) {
    prototype_.reset_ptr();
  }

#if 0
  // This is defined in: fable/schema/magic.hpp
  Map(Type* ptr, std::string&& desc)
      : Map(ptr, make_prototype<T>(), std::move(desc)) {}
#endif

 public:  // Special
  bool unique_properties() const { return unique_properties_; }
  Map<T, P> unique_properties(bool value) && {
    unique_properties_ = value;
    return std::move(*this);
  }

  const std::vector<std::string>& required() const { return required_; }
  Map<T, P> require_properties(const std::vector<std::string>& values) && {
    required_ = values;
    return std::move(*this);
  }
  Map<T, P> require_property(const std::string& value) && {
    required_.emplace_back(value);
    return std::move(*this);
  }

  const std::string& pattern() const { return pattern_; }
  Map<T, P> pattern(const std::string& value) && {
    pattern_ = value;
    return std::move(*this);
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"type", "object"},
        {"additionalProperties", prototype_.json_schema()},
    };
    if (!required_.empty()) {
      j["required"] = required_;
    }
    if (min_properties_ != 0) {
      j["minProperties"] = min_properties_;
    }
    if (max_properties_ != std::numeric_limits<size_t>::max()) {
      j["maxProperties"] = max_properties_;
    }
    if (!pattern_.empty()) {
      j["propertyNames"]["pattern"] = pattern_;
    }
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    this->validate_type(c);
    if (c->size() < min_properties_) {
      this->throw_error(c, "expect at least {} properties, got {}", max_properties_, c->size());
    }
    assert(required_.size() <= max_properties_);
    if (c->size() > max_properties_) {
      this->throw_error(c, "expect at most {} properties, got {}", max_properties_, c->size());
    }
    for (auto& k : required_) {
      c.assert_has(k);
    }

    boost::optional<std::regex> pattern;
    if (!pattern_.empty()) {
      *pattern = std::regex(pattern_);
    }
    for (const auto& kv : c->items()) {
      prototype_.validate(c.at(kv.key()));
      if (pattern && !std::regex_match(kv.key(), *pattern)) {
        this->throw_error(c, "expect property name to match regex '{}': {}", pattern_, kv.key());
      }
    }
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    for (auto& i : c->items()) {
      const auto key = i.key();
      if (unique_properties_ && ptr_->count(key)) {
        this->throw_error(c, "key {} has already been defined", key);
      }
      ptr_->insert(std::make_pair(key, deserialize_item(c, key)));
    }
  }

  Json serialize(const Type& xm) const {
    Json j;
    for (const auto& kv : xm) {
      j[kv.first] = prototype_.serialize(kv.second);
    }
    return j;
  }

  Type deserialize(const Conf& c) const {
    Type tmp;
    for (auto& i : c->items()) {
      const auto key = i.key();
      tmp.insert(std::make_pair(key, deserialize_item(c, key)));
    }
    return tmp;
  }

  T deserialize_item(const Conf& c, const std::string& key) const {
    return prototype_.deserialize(c.at(key));
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  bool unique_properties_{true};
  size_t min_properties_{0};
  size_t max_properties_{std::numeric_limits<size_t>::max()};
  std::string pattern_{};
  std::vector<std::string> required_{};
  PrototypeSchema prototype_{};
  Type* ptr_{nullptr};
};

template <typename T, typename P>
Map<T, P> make_schema(std::map<std::string, T>* ptr, const P& prototype, std::string&& desc) {
  return Map<T, P>(ptr, prototype, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_MAP_HPP_
