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

#include <limits>    // for numeric_limits<>
#include <map>       // for map<>
#include <optional>  // for optional<>
#include <regex>     // for regex, regex_match
#include <string>    // for string
#include <utility>   // for move
#include <vector>    // for vector<>

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable::schema {

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
  using PrototypeSchema = std::remove_cv_t<std::remove_reference_t<P>>;

  Map(Type* ptr, std::string desc) : Map(ptr, make_prototype<T>(), std::move(desc)) {}

  Map(Type* ptr, PrototypeSchema prototype)
      : Base<Map<T, P>>(JsonType::object), prototype_(std::move(prototype)), ptr_(ptr) {
    prototype_.reset_ptr();
  }

  Map(Type* ptr, PrototypeSchema prototype, std::string desc)
      : Base<Map<T, P>>(JsonType::object, std::move(desc)), prototype_(std::move(prototype)), ptr_(ptr) {
    prototype_.reset_ptr();
  }

 public:  // Special
  [[nodiscard]] bool unique_properties() const { return unique_properties_; }
  [[nodiscard]] Map<T, P> unique_properties(bool value) && {
    set_unique_properties(value);
    return std::move(*this);
  }
  void set_unique_properties(bool value) {
    unique_properties_ = value;
  }

  [[nodiscard]] const std::vector<std::string>& required_properties() const { return required_; }
  [[nodiscard]] Map<T, P> require_properties(const std::vector<std::string>& values) && {
    set_required_properties(values);
    return std::move(*this);
  }
  [[nodiscard]] Map<T, P> require_property(const std::string& value) && {
    add_required_property(value);
    return std::move(*this);
  }
  void set_required_properties(const std::vector<std::string>& values) {
    required_ = values;
  }
  void add_required_property(const std::string& value) {
    required_.emplace_back(value);
  }

  [[nodiscard]] const std::string& pattern() const { return pattern_; }
  [[nodiscard]] Map<T, P> pattern(const std::string& value) && {
    pattern_ = value;
    return std::move(*this);
  }
  void set_pattern(const std::string& value) {
    pattern_ = value;
  }

 public:  // Overrides
  [[nodiscard]] Json json_schema() const override {
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

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    if (!this->validate_type(c, err)) {
      return false;
    }

    if (c->size() < min_properties_) {
      return this->set_error(err, c, "expect at least {} properties, got {}", max_properties_, c->size());
    }
    assert(required_.size() <= max_properties_);
    if (c->size() > max_properties_) {
      return this->set_error(err, c, "expect at most {} properties, got {}", max_properties_, c->size());
    }

    for (const auto& k : required_) {
      if (!c.has(k)) {
        return this->set_error(err, c, "missing property: {}", k);
      }
    }

    std::optional<std::regex> pattern;
    if (!pattern_.empty()) {
      try {
        pattern.emplace(pattern_);
      } catch (std::regex_error& e) {
        // NOTE: This is actually a programmer error, and we should probably catch
        // it at the point where the pattern is set.
        return this->set_error(err, c, "invalid regex '{}': {}", pattern_, e.what());
      }
    }
    for (const auto& kv : c->items()) {
      if (!prototype_.validate(c.at(kv.key()), err)) {
        return false;
      }
      if (pattern && !std::regex_match(kv.key(), *pattern)) {
        return this->set_error(err, c, "expect property name to match regex '{}': {}", pattern_, kv.key());
      }
    }
    return true;
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    for (const auto& i : c->items()) {
      const auto& key = i.key();
      if (unique_properties_ && ptr_->count(key)) {
        throw this->error(c, "key {} has already been defined", key);
      }
      ptr_->insert(std::make_pair(key, deserialize_item(c, key)));
    }
  }

  [[nodiscard]] Json serialize(const Type& x) const {
    Json j;
    serialize_into(j, x);
    return j;
  }

  [[nodiscard]] Type deserialize(const Conf& c) const {
    Type tmp;
    deserialize_into(c, tmp);
    return tmp;
  }

  void serialize_into(Json& j, const Type& x) const {
    for (const auto& kv : x) {
      j[kv.first] = prototype_.serialize(kv.second);
    }
  }

  void deserialize_into(const Conf& c, Type& x) const {
    for (const auto& i : c->items()) {
      const auto& key = i.key();
      x.insert(std::make_pair(key, deserialize_item(c, key)));
    }
  }

  [[nodiscard]] T deserialize_item(const Conf& c, const std::string& key) const {
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

template <typename T, typename P, typename S>
Map<T, P> make_schema(std::map<std::string, T>* ptr, P&& prototype, S&& desc) {
  return {ptr, std::forward<P>(prototype), std::forward<S>(desc)};
}

template <typename T, typename S>
Map<T, decltype(make_prototype<T>())> make_schema(std::map<std::string, T>* ptr, S&& desc) {
  return {ptr, std::forward<S>(desc)};
}

}  // namespace fable::schema
