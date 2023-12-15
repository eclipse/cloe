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

#include <initializer_list>  // for initializer_list<>
#include <set>               // for set<>
#include <string>            // for string
#include <type_traits>       // for enable_if_t<>, is_arithmetic<>

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

template <typename T>
class Number : public Base<Number<T>> {
  static_assert(std::is_arithmetic<T>::value, "arithmetic value required");

 public:  // Types and Constructors
  using Type = T;

  template <typename X = T, std::enable_if_t<std::is_integral<X>::value && std::is_unsigned<X>::value, int> = 0>
  Number(Type* ptr, std::string&& desc)
      : Base<Number<T>>(JsonType::number_unsigned, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_integral<X>::value && std::is_signed<X>::value, int> = 0>
  Number(Type* ptr, std::string&& desc)
      : Base<Number<T>>(JsonType::number_integer, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
  Number(Type* ptr, std::string&& desc)
      : Base<Number<T>>(JsonType::number_float, std::move(desc)), ptr_(ptr) {}


 public:  // Special
  T minimum() const { return value_min_; }
  bool exclusive_minimum() const { return exclusive_min_; }
  Number<T> minimum(T value) &&;
  Number<T> exclusive_minimum(T value) &&;

  T maximum() const { return value_max_; }
  bool exclusive_maximum() const { return exclusive_max_; }
  Number<T> maximum(T value) &&;
  Number<T> exclusive_maximum(T value) &&;

  std::pair<T, T> bounds() const;
  Number<T> bounds(T min, T max) &&;
  Number<T> bounds_with(T min, T max, std::initializer_list<T> whitelisted) &&;

  const std::set<T>& whitelist() const { return whitelist_; }
  Number<T> whitelist(T x) &&;
  Number<T> whitelist(std::initializer_list<T> xs) &&;
  void insert_whitelist(T x);

  const std::set<T>& blacklist() const { return blacklist_; }
  Number<T> blacklist(T x) &&;
  Number<T> blacklist(std::initializer_list<T> xs) &&;
  void insert_blacklist(T x);

 public:  // Overrides
  Json json_schema() const override;
  void validate(const Conf& c) const override;
  using Interface::to_json;
  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override;
  Json serialize(const Type& x) const;
  Type deserialize(const Conf& c) const;
  void reset_ptr() override;

 private:
  template <typename B>
  void check_bounds(const Conf& c) const;

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
inline Number<T> make_schema_impl(T* ptr, std::string&& desc) {
  return Number<T>(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable
