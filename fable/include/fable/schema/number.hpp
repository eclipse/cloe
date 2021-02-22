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
 * \file fable/schema/number.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_NUMBER_HPP_
#define FABLE_SCHEMA_NUMBER_HPP_

#include <initializer_list>  // for initializer_list<>
#include <limits>            // for numeric_limits<>
#include <set>               // for set<>
#include <string>            // for string
#include <type_traits>       // for enable_if_t<>, is_arithmetic<>
#include <utility>           // for move
#include <vector>            // for vector<>

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

template <typename T>
class Number : public Base<Number<T>> {
  static_assert(std::is_arithmetic<T>::value, "arithmetic value required");

 public:  // Types and Constructors
  using Type = T;

  template <typename X = T,
            std::enable_if_t<std::is_integral<X>::value && std::is_unsigned<X>::value, int> = 0>
  Number(Type* ptr, std::string&& desc)
      : Base<Number<T>>(JsonType::number_unsigned, std::move(desc)), ptr_(ptr) {}

  template <typename X = T,
            std::enable_if_t<std::is_integral<X>::value && std::is_signed<X>::value, int> = 0>
  Number(Type* ptr, std::string&& desc)
      : Base<Number<T>>(JsonType::number_integer, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
  Number(Type* ptr, std::string&& desc)
      : Base<Number<T>>(JsonType::number_float, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  T minimum() const { return value_min_; }
  bool exclusive_minimum() const { return exclusive_min_; }
  Number<T> minimum(T value) && {
    value_min_ = value;
    exclusive_min_ = false;
    return std::move(*this);
  }
  Number<T> exclusive_minimum(T value) && {
    value_min_ = value;
    exclusive_min_ = true;
    return std::move(*this);
  }

  T maximum() const { return value_max_; }
  bool exclusive_maximum() const { return exclusive_max_; }
  Number<T> maximum(T value) && {
    value_max_ = value;
    exclusive_max_ = false;
    return std::move(*this);
  }
  Number<T> exclusive_maximum(T value) && {
    value_max_ = value;
    exclusive_max_ = true;
    return std::move(*this);
  }

  std::pair<T, T> bounds() const { return std::make_pair(value_min_, value_max_); }
  Number<T> bounds(T min, T max) && {
    exclusive_min_ = false;
    value_min_ = min;
    exclusive_max_ = false;
    value_max_ = max;
    return std::move(*this);
  }
  Number<T> bounds_with(T min, T max, std::initializer_list<T> whitelisted) {
    exclusive_min_ = false;
    value_min_ = min;
    exclusive_max_ = false;
    value_max_ = max;
    for (auto x : whitelisted) {
      insert_whitelist(x);
    }
    return std::move(*this);
  }

  const std::set<T>& whitelist() const { return whitelist_; }
  Number<T> whitelist(T x) && {
    insert_whitelist(x);
    return std::move(*this);
  }
  Number<T> whitelist(std::initializer_list<T> xs) && {
    for (auto x : xs) {
      insert_whitelist(x);
    }
    return std::move(*this);
  }
  void insert_whitelist(T x) {
    if (std::is_floating_point<T>::value) {
      throw std::logic_error("cannot whitelist floating-point numbers");
    }
    if (blacklist_.count(x)) {
      throw std::logic_error("cannot add blacklisted value to whitelist: " + std::to_string(x));
    }
    whitelist_.insert(x);
  }

  const std::set<T>& blacklist() const { return blacklist_; }
  Number<T> blacklist(T x) && {
    insert_blacklist(x);
    return std::move(*this);
  }
  Number<T> blacklist(std::initializer_list<T> xs) && {
    for (auto x : xs) {
      insert_blacklist(x);
    }
    return std::move(*this);
  }
  void insert_blacklist(T x) {
    if (std::is_floating_point<T>::value) {
      throw std::logic_error("cannot blacklist floating-point numbers");
    }
    if (blacklist_.count(x)) {
      throw std::logic_error("cannot add whitelisted value to blacklist: " + std::to_string(x));
    }
    blacklist_.insert(x);
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"type", this->type_string()},
        {exclusive_min_ ? "exclusiveMinimum" : "minimum", value_min_},
        {exclusive_max_ ? "exclusiveMaximum" : "maximum", value_max_},
    };

    if (!std::is_floating_point<T>::value) {
      auto write_list = [&j](auto name, auto xlist) {
        if (!xlist.empty()) {
          std::vector<T> xs;
          for (auto x : xlist) {
            xs.emplace_back(x);
          }
          j[name] = xs;
        }
      };

      write_list("whitelist", whitelist_);
      write_list("blacklist", blacklist_);
    }

    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    switch (c->type()) {
      case JsonType::number_unsigned: {
        check_bounds<uint64_t>(c);
        break;
      }
      case JsonType::number_integer: {
        check_bounds<int64_t>(c);
        break;
      }
      case JsonType::number_float: {
        if (this->type() != JsonType::number_float) {
          this->throw_wrong_type(c);
        }
        check_bounds<double>(c);
        break;
      }
      default:
        this->throw_wrong_type(c);
    }
  }

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

  Type deserialize(const Conf& c) const { return c.get<Type>(); }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  /**
   * Check that the min and max bounds are held by c.
   */
  template <typename B>
  void check_bounds(const Conf& c) const {
    auto v = c.get<B>();

    // Check whitelist and blacklist first.
    if (!std::is_floating_point<T>::value) {
      if (whitelist_.count(v)) {
        return;
      }
      if (blacklist_.count(v)) {
        this->throw_error(c, "unexpected blacklisted value {}", v);
      }
    }

    if (!std::numeric_limits<B>::is_signed && value_min_ < 0) {
      // If B is unsigned and value_min_ is less than 0, there is no way
      // that v cannot fulfill the minimum requirements. Trying to use the
      // other branches will "underflow" the value_min_ which will invalidate
      // any comparison.
    } else if (exclusive_min_) {
      if (v <= static_cast<B>(value_min_)) {
        this->throw_error(c, "expected exclusive minimum of {}, got {}", value_min_, v);
      }
    } else {
      if (v < static_cast<B>(value_min_)) {
        this->throw_error(c, "expected minimum of {}, got {}", value_min_, v);
      }
    }

    if (!std::numeric_limits<B>::is_signed && value_max_ < 0) {
      // If B is unsigned, but our maximum value is somewhere below 0, then v
      // will by definition always be out-of-bounds.
      this->throw_error(c, "expected {}maximum of {}, got {}", (exclusive_max_ ? "exclusive " : ""),
                        value_max_, v);
    } else if (exclusive_max_) {
      if (v >= static_cast<B>(value_max_)) {
        this->throw_error(c, "expected exclusive maximum of {}, got {}", value_max_, v);
      }
    } else {
      if (v > static_cast<B>(value_max_)) {
        this->throw_error(c, "expected maximum of {}, got {}", value_max_, v);
      }
    }
  }

 private:
  bool exclusive_min_{false};
  bool exclusive_max_{false};
  T value_min_{std::numeric_limits<T>::lowest()};
  T value_max_{std::numeric_limits<T>::max()};
  std::set<T> whitelist_{};
  std::set<T> blacklist_{};
  Type* ptr_{nullptr};
};

template <typename T,
          std::enable_if_t<std::is_arithmetic<T>::value && !std::is_enum<T>::value, int> = 0>
inline Number<T> make_schema(T* ptr, std::string&& desc) {
  return Number<T>(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_NUMBER_HPP_
