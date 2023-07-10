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
 * \file fable/schema/array.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <limits>       // for numeric_limits<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_convertible<>
#include <utility>      // for move
#include <vector>       // for vector<>

#include <fable/schema/interface.hpp>  // for Base<>, Box

namespace fable {
namespace schema {

template <typename T, typename P>
class Vector : public Base<Vector<T, P>> {
 public:  // Types and Constructors
  using Type = std::vector<T>;
  using PrototypeSchema = P;

  Vector(Type* ptr, std::string desc) : Vector(ptr, make_prototype<T>(), std::move(desc)) {}

  Vector(Type* ptr, PrototypeSchema prototype)
      : Base<Vector<T, P>>(JsonType::array), prototype_(std::move(prototype)), ptr_(ptr) {}

  Vector(Type* ptr, PrototypeSchema prototype, std::string desc)
      : Base<Vector<T, P>>(JsonType::array, std::move(desc))
      , prototype_(std::move(prototype))
      , ptr_(ptr) {}

 public:  // Specials
  /**
   * Return whether deserialization extends the underlying array (true) or
   * replaces the underlying array (false).
   *
   * By default, this is false.
   */
  bool extend() const { return option_extend_; }

  /**
   * Set whether deserialization should extend the underlying array.
   */
  void set_extend(bool value) { option_extend_ = !value; }

  /**
   * Set whether deserialization should extend the underlying array.
   */
  Vector<T, P> extend(bool value) && {
    option_extend_ = value;
    return std::move(*this);
  }

  size_t min_items() const { return min_items_; }
  void set_min_items(size_t value) { min_items_ = value; }
  Vector<T, P> min_items(size_t value) && {
    min_items_ = value;
    return std::move(*this);
  }

  size_t max_items() const { return max_items_; }
  void set_max_items(size_t value) { max_items_ = value; }
  Vector<T, P> max_items(size_t value) && {
    max_items_ = value;
    return std::move(*this);
  }

 public:  // Overrides
  std::string type_string() const override { return "array of " + prototype_.type_string(); }

  Json json_schema() const override {
    Json j{
        {"type", "array"},
        {"items", prototype_.json_schema()},
    };
    if (min_items_ != 0) {
      j["minItems"] = min_items_;
    }
    if (max_items_ != std::numeric_limits<size_t>::max()) {
      j["maxItems"] = max_items_;
    }
    this->augment_schema(j);
    return j;
  }

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    if (!this->validate_type(c, err)) {
      return false;
    }
    if (c->size() < min_items_) {
      return this->set_error(err, c, "require at least {} items in array, got {}", min_items_,
                             c->size());
    }
    if (c->size() > max_items_) {
      return this->set_error(err, c, "expect at most {} items in array, got {}", max_items_,
                             c->size());
    }
    for (const auto& x : c.to_array()) {
      if (!prototype_.validate(x, err)) {
        return false;
      }
    }
    return true;
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    if (j.type() != JsonType::array) {
      j = Json::array();
    }
    serialize_into(j, *ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    assert(c->type() == this->type_);
    deserialize_into(c, *ptr_);
  }

  Json serialize(const Type& xs) const {
    Json j = Json::array();
    serialize_into(j, xs);
    return j;
  }

  Type deserialize(const Conf& c) const {
    Type vec;
    vec.reserve(c->size());
    fill(vec, c);
    return vec;
  }

  void serialize_into(Json& j, const Type& xs) const {
    for (const auto& x : xs) {
      j.emplace_back(prototype_.serialize(x));
    }
  }

  void deserialize_into(const Conf& c, Type& x) const {
    auto size = c->size();
    if (option_extend_) {
      size += x.size();
    } else {
      x.clear();
    }
    x.reserve(size);
    fill(x, c);
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  void fill(Type& vec, const Conf& c) const {
    for (const auto& x : c.to_array()) {
      T inst = prototype_.deserialize(x);
      vec.emplace_back(std::move(inst));
    }
  }

 private:
  bool option_extend_{false};
  size_t min_items_{0};
  size_t max_items_{std::numeric_limits<size_t>::max()};
  PrototypeSchema prototype_{};
  Type* ptr_{nullptr};
};

template <typename T, typename P>
Vector<T, P> make_schema(std::vector<T>* ptr, P prototype, std::string desc) {
  return Vector<T, P>(ptr, std::move(prototype), std::move(desc));
}

template <typename T>
Vector<T, decltype(make_prototype<T>())> make_schema(std::vector<T>* ptr, std::string desc) {
  return Vector<T, decltype(make_prototype<T>())>(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable
