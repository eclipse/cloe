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

#include <memory>   // for shared_ptr<>
#include <string>   // for string
#include <utility>  // for move

#include <fable/fable_fwd.hpp>         // for Confable
#include <fable/schema/interface.hpp>  // for Interface

namespace fable::schema {

template <typename T, std::enable_if_t<std::is_base_of_v<Confable, T>, int> = 0>
class FromConfable : public Base<FromConfable<T>> {
 public:  // Types and Constructors
  using Type = T;

  explicit FromConfable(std::string desc = "")
      : Base<FromConfable<T>>(std::move(desc)), schema_(Type().schema()) {
    schema_.reset_ptr();  // type was temporary
    this->type_ = schema_.type();
  }

  FromConfable(Type* ptr, std::string desc)
      : Base<FromConfable<Type>>(ptr->schema().type(), std::move(desc))
      , schema_(ptr->schema())
      , ptr_(ptr) {
    assert(ptr != nullptr);
  }

 public:  // Special
  [[nodiscard]] Box get_confable_schema() const { return schema_.clone(); }

 public:  // Overrides
  [[nodiscard]] Json json_schema() const override {
    Json j = schema_.json_schema();
    this->augment_schema(j);
    return j;
  }

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    if (ptr_ == nullptr) {
      return schema_.validate(c, err);
    }
    return ptr_->validate(c, err);
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    ptr_->to_json(j);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    ptr_->from_conf(c);
  }

  [[nodiscard]] Json serialize(const Type& x) const { return x.to_json(); }

  [[nodiscard]] Type deserialize(const Conf& c) const {
    Type tmp;
    tmp.from_conf(c);
    return tmp;
  }

  void serialize_into(Json& j, const Type& x) const { x.to_json(j); }

  void deserialize_into(const Conf& c, Type& x) const { x.from_conf(c); }

  void reset_ptr() override {
    ptr_ = nullptr;
    schema_.reset_ptr();
  }

 private:
  Box schema_;
  Type* ptr_{nullptr};
};

template <typename T>
FromConfable<T> make_schema(T* ptr, std::string desc) {
  assert(ptr != nullptr);
  return FromConfable<T>(ptr, std::move(desc));
}

}  // namespace fable::schema
