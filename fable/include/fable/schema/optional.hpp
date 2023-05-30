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
 * \file fable/schema/optional.hpp
 * \see  fable/schema/optional_test.cpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <optional>     // for optional<>
#include <string>       // for string
#include <type_traits>  // for is_same_v<>, enable_if<>
#include <utility>      // for move

#include <fable/schema/interface.hpp>  // for Base<>, Box
#include <fable/utility/optional.hpp>  // for adl_serializer<>

namespace fable {
namespace schema {

/**
 * Helper type trait class to use with std::enable_if and friends.
 *
 * The value `is_optional<T>::value` is true if T is one of:
 * - std::optional
 * - boost::optional, if boost_optional.hpp is included
 *
 * \see fable/schema/boost_optional.hpp
 */
template <typename T>
struct is_optional : std::false_type {};

template <typename X>
struct is_optional<std::optional<X>> : std::true_type {};

/**
 * Optional de-/serializes a value that can be null.
 *
 * In a JSON object, a field that has the value null is not the
 * same thing as a field that is missing!
 *
 * Optional is a template class that supports both:
 * - std::optional
 * - boost::optional, if boost_optional.hpp is included
 */
template <typename T, typename P>
class Optional : public Base<Optional<T, P>> {
  static_assert(is_optional<T>::value);

 public:  // Types and Constructors
  using Type = T;
  using ValueType = typename Type::value_type;
  using PrototypeSchema = P;

  Optional(Type* ptr, std::string&& desc);
  Optional(Type* ptr, const PrototypeSchema& prototype, std::string&& desc)
      : Base<Optional<T, P>>(prototype.type(), std::move(desc)), prototype_(prototype), ptr_(ptr) {
    prototype_.reset_ptr();
  }

#if 0
  // This is defined in: fable/schema/xmagic.hpp
  Optional(T* ptr, std::string&& desc)
    : Optional<T, P>(ptr, make_prototype<typename T::value_type>(), std::move(desc)) {}
#endif

 public:  // Overrides
  std::string type_string() const override { return prototype_.type_string() + "?"; }
  bool is_variant() const override { return true; }

  Json json_schema() const override {
    Json j{{
        "oneOf",
        {
            Json{
                {"type", "null"},
            },
            prototype_.json_schema(),
        },
    }};
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    if (c->type() == JsonType::null) {
      return;
    }
    this->validate_type(c);
    prototype_.validate(c);
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

  Json serialize(const Type& x) const {
    if (x) {
      return prototype_.serialize(x.value());
    } else {
      return nullptr;
    }
  }

  Type deserialize(const Conf& c) const {
    if (c->type() == JsonType::null) {
      return Type{};
    }
    return prototype_.deserialize(c);
  }

  void serialize_into(Json& j, const Type& x) const { j = serialize(x); }

  void deserialize_into(const Conf& c, Type& x) const { x = deserialize(c); }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  PrototypeSchema prototype_;
  Type* ptr_{nullptr};
};

// Define make_schema only for std::optional and boost::optional.
template <typename T, typename P, std::enable_if_t<is_optional<T>::value, bool> = true>
inline Optional<T, P> make_schema(T* ptr, const P& prototype, std::string&& desc) {
  return Optional<T, P>(ptr, prototype, std::move(desc));
}

}  // namespace schema
}  // namespace fable
