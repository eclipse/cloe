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
 * \file fable/schema/duration.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 * \see  fable/schema/number.hpp
 */

#pragma once

#include <chrono>   // for duration<>
#include <limits>   // for numeric_limits<>
#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>   // for Base<>
#include <fable/utility/templates.hpp>  // for is_safe_cast<>, typeinfo<>

namespace fable::schema {

template <typename T, typename Period>
class Duration : public Base<Duration<T, Period>> {
 public:  // Types and Constructors
  using Type = std::chrono::duration<T, Period>;

  template <typename X = T,
            std::enable_if_t<std::is_integral_v<X> && std::is_unsigned_v<X>, int> = 0>
  Duration(Type* ptr, std::string desc)
      : Base<Duration<T, Period>>(JsonType::number_unsigned, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_integral_v<X> && std::is_signed_v<X>, int> = 0>
  Duration(Type* ptr, std::string desc)
      : Base<Duration<T, Period>>(JsonType::number_integer, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_floating_point_v<X>, int> = 0>
  Duration(Type* ptr, std::string desc)
      : Base<Duration<T, Period>>(JsonType::number_float, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  [[nodiscard]] T minimum() const { return value_min_; }
  [[nodiscard]] bool exclusive_minimum() const { return exclusive_min_; }
  Duration<T, Period> minimum(T value) && {
    value_min_ = value;
    exclusive_min_ = false;
    return std::move(*this);
  }
  Duration<T, Period> exclusive_minimum(T value) && {
    value_min_ = value;
    exclusive_min_ = true;
    return std::move(*this);
  }

  [[nodiscard]] T maximum() const { return value_max_; }
  [[nodiscard]] bool exclusive_maximum() const { return exclusive_max_; }
  Duration<T, Period> maximum(T value) && {
    value_max_ = value;
    exclusive_max_ = false;
    return std::move(*this);
  }
  Duration<T, Period> exclusive_maximum(T value) && {
    value_max_ = value;
    exclusive_max_ = true;
    return std::move(*this);
  }

  [[nodiscard]] std::pair<T, T> bounds() const { return std::make_pair(value_min_, value_max_); }
  Duration<T, Period> bounds(T min, T max) && {
    exclusive_min_ = false;
    value_min_ = min;
    exclusive_max_ = false;
    value_max_ = max;
    return std::move(*this);
  }

 public:  // Overrides
  [[nodiscard]] Json json_schema() const override {
    Json j{
        {"type", this->type_string()},
        {exclusive_min_ ? "exclusiveMinimum" : "minimum", value_min_},
        {exclusive_max_ ? "exclusiveMaximum" : "maximum", value_max_},
    };
    this->augment_schema(j);
    return j;
  }

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
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

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = deserialize(c);
  }

  [[nodiscard]] Json serialize(const Type& x) const { return x.count(); }

  [[nodiscard]] Type deserialize(const Conf& c) const { return Type(c.get<T>()); }

  void serialize_into(Json& j, const Type& x) const { j = x.count(); }

  void deserialize_into(const Conf& c, Type& x) const { x = deserialize(c); }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  /**
   * Check that the min and max bounds are held by c.
   */
  template <typename B>
  bool validate_bounds(const Conf& c, std::optional<SchemaError>& err) const {
    auto original = c.get<B>();
    auto value = static_cast<T>(original);
    if constexpr (!std::is_floating_point_v<T>) {
      if (!is_cast_safe<T>(original)) {
        return this->set_error(err, c,
                               "failed to convert input to destination type {}, got {}( {} ) = {}",
                               typeinfo<T>::name, typeinfo<B>::name, original, value);
      }
    }

    // Check minimum value:
    if (exclusive_min_) {
      if (value <= value_min_) {
        return this->set_error(err, c, "expected exclusive minimum of {}, got {}", value_min_,
                               value);
      }
    } else {
      if (value < value_min_) {
        return this->set_error(err, c, "expected minimum of {}, got {}", value_min_, value);
      }
    }

    if (exclusive_max_) {
      if (value >= value_max_) {
        return this->set_error(err, c, "expected exclusive maximum of {}, got {}", value_max_,
                               value);
      }
    } else {
      if (value > value_max_) {
        return this->set_error(err, c, "expected maximum of {}, got {}", value_max_, value);
      }
    }

    return true;
  }

 private:
  bool exclusive_min_{false};
  bool exclusive_max_{false};
  T value_min_{std::numeric_limits<T>::lowest()};
  T value_max_{std::numeric_limits<T>::max()};
  Type* ptr_{nullptr};
};

template <typename Rep, typename Period>
inline Duration<Rep, Period> make_schema(std::chrono::duration<Rep, Period>* ptr,
                                         std::string desc) {
  return Duration<Rep, Period>(ptr, std::move(desc));
}

}  // namespace fable::schema
