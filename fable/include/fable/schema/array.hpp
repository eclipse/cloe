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
 * \see  fable/schema/magic.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_ARRAY_HPP_
#define FABLE_SCHEMA_ARRAY_HPP_

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
class Array : public Base<Array<T, P>> {
 public:  // Types and Constructors
  using Type = std::vector<T>;
  using PrototypeSchema = P;

  Array(Type* ptr, std::string&& desc);
  Array(Type* ptr, const PrototypeSchema& prototype)
      : Base<Array<T, P>>(JsonType::array), prototype_(prototype), ptr_(ptr) {}
  Array(Type* ptr, const PrototypeSchema& prototype, std::string&& desc)
      : Base<Array<T, P>>(JsonType::array, std::move(desc)), prototype_(prototype), ptr_(ptr) {}

#if 0
  // This is defined in: fable/schema/magic.hpp
  Array(Type* ptr, std::string&& desc)
      : Array(ptr, make_prototype<T>(), std::move(desc)) {}
#endif

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
  Array<T, P> extend(bool value) && {
    option_extend_ = value;
    return std::move(*this);
  }

  size_t min_items() const { return min_items_; }
  void set_min_items(size_t value) { min_items_ = value; }
  Array<T, P> min_items(size_t value) && {
    min_items_ = value;
    return std::move(*this);
  }

  size_t max_items() const { return max_items_; }
  void set_max_items(size_t value) { max_items_ = value; }
  Array<T, P> max_items(size_t value) && {
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
      j["maxitems"] = max_items_;
    }
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    this->validate_type(c);
    if (c->size() < min_items_) {
      this->throw_error(c, "require at least {} items in array, got {}", min_items_, c->size());
    }
    if (c->size() > max_items_) {
      this->throw_error(c, "expect at most {} items in array, got {}", max_items_, c->size());
    }

    for (const auto& x : c.to_array()) {
      prototype_.validate(x);
    }
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    assert(c->type() == this->type_);

    if (!option_extend_) {
      ptr_->clear();
    }

    ptr_->reserve(c->size() + ptr_->size());
    fill(*ptr_, c);
  }

  Json serialize(const Type& xs) const {
    Json j = Json::array();
    for (const auto& x : xs) {
      j.emplace_back(prototype_.serialize(x));
    }
    return j;
  }

  Type deserialize(const Conf& c) const {
    Type vec;
    vec.reserve(c->size());
    fill(vec, c);
    return vec;
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
Array<T, P> make_schema(std::vector<T>* ptr, const P& prototype, std::string&& desc) {
  return Array<T, P>(ptr, prototype, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_ARRAY_HPP_
