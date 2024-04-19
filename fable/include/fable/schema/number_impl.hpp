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

#include <fable/schema/number.hpp>      // for Number<>
#include <fable/utility/templates.hpp>  // for is_safe_cast<>, typeinfo<>

namespace fable::schema {

template <typename T>
Number<T> Number<T>::minimum(T value) && {
  set_minimum(value);
  return std::move(*this);
}

template <typename T>
void Number<T>::set_minimum(T value) {
  value_min_ = value;
  exclusive_min_ = false;
}

template <typename T>
Number<T> Number<T>::exclusive_minimum(T value) && {
  set_exclusive_minimum(value);
  return std::move(*this);
}

template <typename T>
void Number<T>::set_exclusive_minimum(T value) {
  value_min_ = value;
  exclusive_min_ = true;
}

template <typename T>
Number<T> Number<T>::maximum(T value) && {
  set_maximum(value);
  return std::move(*this);
}

template <typename T>
void Number<T>::set_maximum(T value) {
  value_max_ = value;
  exclusive_max_ = false;
}

template <typename T>
Number<T> Number<T>::exclusive_maximum(T value) && {
  set_exclusive_maximum(value);
  return std::move(*this);
}

template <typename T>
void Number<T>::set_exclusive_maximum(T value) {
  value_max_ = value;
  exclusive_max_ = true;
}

template <typename T>
std::pair<T, T> Number<T>::bounds() const {
  return std::make_pair(value_min_, value_max_);
}

template <typename T>
Number<T> Number<T>::bounds(T min, T max) && {
  set_bounds(min, max);
  return std::move(*this);
}

template <typename T>
void Number<T>::set_bounds(T min, T max) {
  exclusive_min_ = false;
  value_min_ = min;
  exclusive_max_ = false;
  value_max_ = max;
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
  extend_whitelist(std::move(xs));
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
void Number<T>::extend_whitelist(std::initializer_list<T> xs) {
  for (const auto& x : xs) {
    insert_whitelist(x);
  }
}

template <typename T>
void Number<T>::reset_whitelist(std::initializer_list<T> xs) {
  whitelist_.clear();
  extend_whitelist(std::move(xs));
}

template <typename T>
Number<T> Number<T>::blacklist(T x) && {
  insert_blacklist(x);
  return std::move(*this);
}

template <typename T>
Number<T> Number<T>::blacklist(std::initializer_list<T> xs) && {
  extend_blacklist(std::move(xs));
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
void Number<T>::extend_blacklist(std::initializer_list<T> xs) {
  for (const auto& x : xs) {
    insert_blacklist(x);
  }
}

template <typename T>
void Number<T>::reset_blacklist(std::initializer_list<T> xs) {
  blacklist_.clear();
  extend_blacklist(std::move(xs));
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
bool Number<T>::validate(const Conf& c, std::optional<SchemaError>& err) const {
  switch (c->type()) {
    case JsonType::number_unsigned:
      return validate_bounds<uint64_t>(c, err);
    case JsonType::number_integer:
      return validate_bounds<int64_t>(c, err);
    case JsonType::number_float:
      if (this->type() != JsonType::number_float) {
        return this->set_wrong_type(err, c);
      } else {
        return validate_bounds<double>(c, err);
      }
    default:
      return this->set_wrong_type(err, c);
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
Json Number<T>::serialize(const T& x) const {
  return x;
}

template <typename T>
T Number<T>::deserialize(const Conf& c) const {
  return c.get<T>();
}

template <typename T>
void Number<T>::serialize_into(Json& j, const T& x) const {
  j = x;
}

template <typename T>
void Number<T>::deserialize_into(const Conf& c, T& x) const {
  x = c.get<T>();
}

template <typename T>
void Number<T>::reset_ptr() {
  ptr_ = nullptr;
}

/**
 * Check that the min and max bounds are held by c.
 *
 * Use-cases with the bounds T={} and the input B=[]:
 * (a) ---{---[--0--]---}--- B within T
 *     Note that this doesn't mean we can stop looking, since the actual range
 *     might be in the top or bottom of T:
 *     -------[--0--]{--}---
 *
 * (b) ---[---{--0--}---]--- T within B
 *
 * (c) ---[------{------]--} T overlaps B
 */
template <typename T>
template <typename B>
bool Number<T>::validate_bounds(const Conf& c, std::optional<SchemaError>& err) const {
  auto original = c.get<B>();
  auto value = static_cast<T>(original);
  if constexpr (!std::is_floating_point_v<T>) {
    if (!is_cast_safe<T>(original)) {
      return this->set_error(err, c,
                             "failed to convert input to destination type {}, got {}( {} ) = {}",
                             typeinfo<T>::name, typeinfo<B>::name, original, value);
    }
  }

  // Check whitelist and blacklist first:
  if (!std::is_floating_point_v<T>) {
    if (whitelist_.count(value)) {
      return true;
    }
    if (blacklist_.count(value)) {
      return this->set_error(err, c, "unexpected blacklisted value {}", value);
    }
  }

  // Check minimum value:
  if (exclusive_min_) {
    if (value <= value_min_) {
      return this->set_error(err, c, "expected exclusive minimum > {}, got {}", value_min_, value);
    }
  } else {
    if (value < value_min_) {
      return this->set_error(err, c, "expected minimum >= {}, got {}", value_min_, value);
    }
  }

  if (exclusive_max_) {
    if (value >= value_max_) {
      return this->set_error(err, c, "expected exclusive maximum < {}, got {}", value_max_, value);
    }
  } else {
    if (value > value_max_) {
      return this->set_error(err, c, "expected maximum <= {}, got {}", value_max_, value);
    }
  }

  return true;
}

}  // namespace fable::schema
