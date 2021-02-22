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
#ifndef FABLE_SCHEMA_OPTIONAL_HPP_
#define FABLE_SCHEMA_OPTIONAL_HPP_

#include <string>   // for string
#include <utility>  // for move

#include <boost/optional.hpp>  // for optional<>

#include <fable/json/with_boost.hpp>   // for to_json, from_json
#include <fable/schema/interface.hpp>  // for Base<>, Box

namespace fable {
namespace schema {

template <typename T, typename P>
class Optional : public Base<Optional<T, P>> {
 public:  // Types and Constructors
  using Type = boost::optional<T>;
  using PrototypeSchema = P;

  Optional(Type* ptr, std::string&& desc);
  Optional(Type* ptr, const PrototypeSchema& prototype, std::string&& desc)
      : Base<Optional<T, P>>(prototype.type(), std::move(desc)), prototype_(prototype), ptr_(ptr) {
    prototype_.reset_ptr();
  }

#if 0
  // This is defined in: fable/schema/magic.hpp
  Optional(Type* ptr, std::string&& desc)
      : Optional(ptr, make_prototype<T>(), std::move(desc)) {}
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
      return prototype_.serialize(x.get());
    } else {
      return nullptr;
    }
  }

  Type deserialize(const Conf& c) const {
    if (c->type() == JsonType::null) {
      return boost::none;
    }
    return prototype_.deserialize(c);
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  PrototypeSchema prototype_;
  Type* ptr_{nullptr};
};

template <typename T, typename P>
Optional<T, P> make_schema(boost::optional<T>* ptr, const P& prototype, std::string&& desc) {
  return Optional<T, P>(ptr, prototype, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_OPTIONAL_HPP_
