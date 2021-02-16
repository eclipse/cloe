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
 * \file fable/schema/json.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_JSON_HPP_
#define FABLE_SCHEMA_JSON_HPP_

#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

/**
 * FromJson uses the `from_json` and `to_json` methods for deserializing and
 * deserializing data to the type.
 *
 * This provides only minimal validation, and its use is not recommended.
 * It is only included because sometimes we have little choice but to work
 * with what is laid before us.
 *
 * Note that the constructor is different to most other schema types: it
 * requires you to specify the expected JSON type. This incidentally means
 * that it is also very hard to use this schema type by accident.
 */
template <typename T>
class FromJson : public Base<FromJson<T>> {
 public:  // Types and Constructors
  using Type = T;

  FromJson(Type* ptr, JsonType t, std::string&& desc)
      : Base<FromJson<T>>(t, std::move(desc)), ptr_(ptr) {}

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"type", this->type_string()},
    };
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override { this->validate_type(c); }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = static_cast<const Type&>(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = c.get<Type>();
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  Type* ptr_{nullptr};
};

template <typename T>
inline FromJson<T> make_schema(T* ptr, JsonType t, std::string&& desc) {
  return FromJson<T>(ptr, t, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_JSON_HPP_
