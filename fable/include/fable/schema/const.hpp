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
 * \file fable/schema/const.hpp
 * \see  fable/schema/const_test.cpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Base<>, Box
#include <fable/schema/string.hpp>     // for String

namespace fable {
namespace schema {

template <typename T, typename P>
class Const : public Base<Const<T, P>> {
 public:  // Types and Constructors
  using Type = T;
  using PrototypeSchema = P;

  Const(Type constant, std::string desc)
      : Const(std::move(constant), make_prototype<Type>(), std::move(desc)) {}

  Const(Type constant, PrototypeSchema prototype, std::string desc)
      : Base<Const<T, P>>(prototype.type(), std::move(desc))
      , prototype_(std::move(prototype))
      , constant_(std::move(constant)) {
    prototype_.reset_ptr();
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"const", constant_},
    };
    this->augment_schema(j);
    return j;
  }

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    Type tmp = prototype_.deserialize(c);
    if (tmp != constant_) {
      return this->set_error(err, c, "expected const value {}, got {}", constant_, tmp);
    }
    return true;
  }

  using Interface::to_json;
  void to_json(Json& j) const override { j = serialize(constant_); }

  void from_conf(const Conf& c) override { this->validate_or_throw(c); }

  Json serialize(const Type& x) const { return prototype_.serialize(x); }

  Type deserialize(const Conf& c) const {
    this->validate_or_throw(c);
    return constant_;
  }

  void serialize_into(Json& j, const Type& x) const { prototype_.serialize_into(j, x); }

  void deserialize_into(const Conf& c, Type& x) const {
    this->validate_or_throw(c);
    x = constant_;
  }

  void reset_ptr() override {}

 private:
  PrototypeSchema prototype_;
  const Type constant_;
};

template <typename T, typename P>
Const<T, P> make_const_schema(T constant, P prototype, std::string desc) {
  return Const<T, P>(std::move(constant), std::move(prototype), std::move(desc));
}

template <typename T>
Const<T, decltype(make_prototype<T>())> make_const_schema(T constant, std::string desc) {
  return Const<T, decltype(make_prototype<T>())>(std::move(constant), std::move(desc));
}

}  // namespace schema
}  // namespace fable
