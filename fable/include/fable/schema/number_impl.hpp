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
 * \file fable/schema/number_impl.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 *
 * You should only include this file if you need to use a type that is
 * numeric and not one of the standard primitives.
 */

#pragma once

#include <initializer_list>  // for initializer_list<>
#include <limits>            // for numeric_limits<>
#include <set>               // for set<>
#include <string>            // for string
#include <type_traits>       // for enable_if_t<>, is_arithmetic<>
#include <utility>           // for move
#include <vector>            // for vector<>

#include <fable/schema/number.hpp>  // for Number<>

namespace fable {
namespace schema {

template <typename T>
Number<T> Number<T>::minimum(T value) && {
  value_min_ = value;
  exclusive_min_ = false;
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::exclusive_minimum(T value) && {
  value_min_ = value;
  exclusive_min_ = true;
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::maximum(T value) && {
  value_max_ = value;
  exclusive_max_ = false;
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::exclusive_maximum(T value) && {
  value_max_ = value;
  exclusive_max_ = true;
  return std::move(*this);
}

template <typename T>
std::pair<T, T> Number<T>::bounds() const { return std::make_pair(value_min_, value_max_); }

template <typename T>
Number<T> Number<T>::bounds(T min, T max) && {
  exclusive_min_ = false;
  value_min_ = min;
  exclusive_max_ = false;
  value_max_ = max;
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::bounds_with(T min, T max, std::initializer_list<T> whitelisted) && {
  exclusive_min_ = false;
  value_min_ = min;
  exclusive_max_ = false;
  value_max_ = max;
  for (auto x : whitelisted) {
    insert_whitelist(x);
  }
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::whitelist(T x) && {
  insert_whitelist(x);
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::whitelist(std::initializer_list<T> xs) && {
  for (auto x : xs) {
    insert_whitelist(x);
  }
  return std::move(*this);
}

template <typename T>
void Number<T>::insert_whitelist(T x) {
  if (std::is_floating_point_v<T>) {
    throw std::logic_error("cannot whitelist floating-point numbers");
  }
  if (blacklist_.count(x)) {
    throw std::logic_error("cannot add blacklisted value to whitelist: " + std::to_string(x));
  }
  whitelist_.insert(x);
}

template <typename T>
Number<T> Number<T>::blacklist(T x) && {
  insert_blacklist(x);
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::blacklist(std::initializer_list<T> xs) && {
  for (auto x : xs) {
    insert_blacklist(x);
  }
  return std::move(*this);
}

template <typename T>
void Number<T>::insert_blacklist(T x) {
  if (std::is_floating_point_v<T>) {
    throw std::logic_error("cannot blacklist floating-point numbers");
  }
  if (blacklist_.count(x)) {
    throw std::logic_error("cannot add whitelisted value to blacklist: " + std::to_string(x));
  }
  blacklist_.insert(x);
}

template <typename T>
Json Number<T>::json_schema() const {
  Json j{
      {"type", this->type_string()},
      {exclusive_min_ ? "exclusiveMinimum" : "minimum", value_min_},
      {exclusive_max_ ? "exclusiveMaximum" : "maximum", value_max_},
  };

  if (!std::is_floating_point_v<T>) {
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

template <typename T>
void Number<T>::validate(const Conf& c) const {
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

template <typename T>
void Number<T>::to_json(Json& j) const {
  assert(ptr_ != nullptr);
  j = serialize(*ptr_);
}

template <typename T>
void Number<T>::from_conf(const Conf& c) {
  assert(ptr_ != nullptr);
  *ptr_ = deserialize(c);
}

template <typename T>
Json Number<T>::serialize(const T& x) const { return x; }

template <typename T>
T Number<T>::deserialize(const Conf& c) const { return c.get<T>(); }

template <typename T>
void Number<T>::serialize_into(Json& j, const T& x) const { j = x; }

template <typename T>
void Number<T>::deserialize_into(const Conf& c, T& x) const { x = c.get<T>(); }

template <typename T>
void Number<T>::reset_ptr() { ptr_ = nullptr; }

/**
 * Check that the min and max bounds are held by c.
 */
template <typename T>
template <typename B>
void Number<T>::check_bounds(const Conf& c) const {
  auto v = c.get<B>();

  // Check whitelist and blacklist first.
  if (!std::is_floating_point_v<T>) {
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

}  // namespace schema
}  // namespace fable
