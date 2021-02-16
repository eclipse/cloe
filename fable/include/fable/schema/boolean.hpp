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
 * \file fable/schema/boolean.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_BOOLEAN_HPP_
#define FABLE_SCHEMA_BOOLEAN_HPP_

#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

class Boolean : public Base<Boolean> {
 public:  // Types and Constructors
  using Type = bool;

  Boolean(Type* ptr, std::string&& desc) : Base(JsonType::boolean, std::move(desc)), ptr_(ptr) {}

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"type", "boolean"},
    };
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override { this->validate_type(c); }

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
  Type* ptr_{nullptr};
};

inline Boolean make_schema(bool* ptr, std::string&& desc) { return Boolean(ptr, std::move(desc)); }

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_BOOLEAN_HPP_
