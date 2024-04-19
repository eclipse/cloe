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

namespace fable::schema {

template <typename T>
class Number : public Base<Number<T>> {
  static_assert(std::is_arithmetic_v<T>, "arithmetic value required");

 public:  // Types and Constructors
  using Type = T;

  template <typename X = T, std::enable_if_t<std::is_integral_v<X> && std::is_unsigned_v<X>, int> = 0>
  Number(Type* ptr, std::string desc)
      : Base<Number<T>>(JsonType::number_unsigned, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_integral_v<X> && std::is_signed_v<X>, int> = 0>
  Number(Type* ptr, std::string desc)
      : Base<Number<T>>(JsonType::number_integer, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_floating_point_v<X>, int> = 0>
  Number(Type* ptr, std::string desc)
      : Base<Number<T>>(JsonType::number_float, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  [[nodiscard]] T minimum() const { return value_min_; }
  void set_minimum(T value);
  [[nodiscard]] Number<T> minimum(T value) &&;

  [[nodiscard]] bool exclusive_minimum() const { return exclusive_min_; }
  void set_exclusive_minimum(T value);
  [[nodiscard]] Number<T> exclusive_minimum(T value) &&;

  [[nodiscard]] T maximum() const { return value_max_; }
  void set_maximum(T value);
  [[nodiscard]] Number<T> maximum(T value) &&;

  [[nodiscard]] bool exclusive_maximum() const { return exclusive_max_; }
  void set_exclusive_maximum(T value);
  [[nodiscard]] Number<T> exclusive_maximum(T value) &&;

  [[nodiscard]] std::pair<T, T> bounds() const;
  void set_bounds(T min, T max);
  [[nodiscard]] Number<T> bounds(T min, T max) &&;

  void set_bounds_with(T min, T max, std::initializer_list<T> whitelisted);
  [[nodiscard]] Number<T> bounds_with(T min, T max, std::initializer_list<T> whitelisted) &&;

  [[nodiscard]] const std::set<T>& whitelist() const { return whitelist_; }
  void insert_whitelist(T x);
  [[nodiscard]] Number<T> whitelist(T x) &&;
  void reset_whitelist(std::initializer_list<T> xs = {});
  void extend_whitelist(std::initializer_list<T> xs);
  [[nodiscard]] Number<T> whitelist(std::initializer_list<T> xs) &&;

  [[nodiscard]] const std::set<T>& blacklist() const { return blacklist_; }
  void insert_blacklist(T x);
  [[nodiscard]] Number<T> blacklist(T x) &&;
  void reset_blacklist(std::initializer_list<T> xs = {});
  void extend_blacklist(std::initializer_list<T> xs);
  [[nodiscard]] Number<T> blacklist(std::initializer_list<T> xs) &&;

 public:  // Overrides
  [[nodiscard]] Json json_schema() const override;
  bool validate(const Conf& c, std::optional<SchemaError>& err) const override;
  using Interface::to_json;
  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override;
  [[nodiscard]] Json serialize(const Type& x) const;
  [[nodiscard]] Type deserialize(const Conf& c) const;
  void serialize_into(Json& j, const Type& x) const;
  void deserialize_into(const Conf& c, Type& x) const;
  void reset_ptr() override;

 private:
  template <typename B>
  bool validate_bounds(const Conf& c, std::optional<SchemaError>& err) const;

 private:
  bool exclusive_min_{false};
  bool exclusive_max_{false};
  T value_min_{std::numeric_limits<T>::lowest()};
  T value_max_{std::numeric_limits<T>::max()};
  std::set<T> whitelist_{};
  std::set<T> blacklist_{};
  Type* ptr_{nullptr};
};

template <typename T, typename S, std::enable_if_t<std::is_arithmetic_v<T> && !std::is_enum_v<T>, int> = 0>
Number<T> make_schema(T* ptr, S&& desc) {
  return {ptr, std::forward<S>(desc)};
}

}  // namespace fable::schema
