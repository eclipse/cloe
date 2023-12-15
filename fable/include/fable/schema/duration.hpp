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

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

template <typename T, typename Period>
class Duration : public Base<Duration<T, Period>> {
 public:  // Types and Constructors
  using Type = std::chrono::duration<T, Period>;

  template <typename X = T,
            std::enable_if_t<std::is_integral<X>::value && std::is_unsigned<X>::value, int> = 0>
  Duration(Type* ptr, std::string&& desc)
      : Base<Duration<T, Period>>(JsonType::number_unsigned, std::move(desc)), ptr_(ptr) {}

  template <typename X = T,
            std::enable_if_t<std::is_integral<X>::value && std::is_signed<X>::value, int> = 0>
  Duration(Type* ptr, std::string&& desc)
      : Base<Duration<T, Period>>(JsonType::number_integer, std::move(desc)), ptr_(ptr) {}

  template <typename X = T, std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
  Duration(Type* ptr, std::string&& desc)
      : Base<Duration<T, Period>>(JsonType::number_float, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  T minimum() const { return value_min_; }
  bool exclusive_minimum() const { return exclusive_min_; }
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

  T maximum() const { return value_max_; }
  bool exclusive_maximum() const { return exclusive_max_; }
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

  std::pair<T, T> bounds() const { return std::make_pair(value_min_, value_max_); }
  Duration<T, Period> bounds(T min, T max) && {
    exclusive_min_ = false;
    value_min_ = min;
    exclusive_max_ = false;
    value_max_ = max;
    return std::move(*this);
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"type", this->type_string()},
        {exclusive_min_ ? "exclusiveMinimum" : "minimum", value_min_},
        {exclusive_max_ ? "exclusiveMaximum" : "maximum", value_max_},
    };
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

  Json serialize(const Type& x) const { return x.count(); }

  Type deserialize(const Conf& c) const { return Type(c.get<T>()); }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  /**
   * Check that the min and max bounds are held by c.
   */
  template <typename B>
  void check_bounds(const Conf& c) const {
    auto v = c.get<B>();
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
  Type* ptr_{nullptr};
};

template <typename Rep, typename Period>
inline Duration<Rep, Period> make_schema_impl(std::chrono::duration<Rep, Period>* ptr,
                                         std::string&& desc) {
  return Duration<Rep, Period>(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable
