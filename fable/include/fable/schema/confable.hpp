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
 * \file fable/schema/confable.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 * \see  fable/confable.hpp
 */

#pragma once
#ifndef FABLE_SCHEMA_CONFABLE_HPP_
#define FABLE_SCHEMA_CONFABLE_HPP_

#include <memory>   // for shared_ptr<>
#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Interface

namespace fable {

// Forward declarations:
class Confable;

namespace schema {

template <typename T, std::enable_if_t<std::is_base_of<Confable, T>::value, int> = 0>
class FromConfable : public Base<FromConfable<T>> {
 public:  // Types and Constructors
  using Type = T;

  explicit FromConfable(std::string&& desc = "") {
    schema_ = T().schema();
    schema_.reset_ptr();
    this->type_ = schema_.type();
    this->desc_ = std::move(desc);
  }

  FromConfable(Type* ptr, std::string&& desc)
      : Base<FromConfable<T>>(ptr->schema().type(), std::move(desc))
      , schema_(ptr->schema())
      , ptr_(ptr) {
    assert(ptr != nullptr);
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j = schema_.json_schema();
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    if (ptr_ == nullptr) {
      schema_.validate(c);
    } else {
      ptr_->validate(c);
    }
  }

  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    ptr_->to_json(j);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    ptr_->from_conf(c);
  }

  Json serialize(const Type& x) const { return x.to_json(); }

  Type deserialize(const Conf& c) const {
    T tmp;
    tmp.from_conf(c);
    return tmp;
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  Box schema_;
  Type* ptr_{nullptr};
};

template <typename T>
FromConfable<T> make_schema(T* ptr, std::string&& desc) {
  assert(ptr != nullptr);
  return FromConfable<T>(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_CONFABLE_HPP_
