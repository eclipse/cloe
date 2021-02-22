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
 * \file fable/schema/passthru.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_PASSTHRU_HPP_
#define FABLE_SCHEMA_PASSTHRU_HPP_

#include <string>   // for string
#include <utility>  // for move

#include <boost/optional.hpp>  // for optional<>

#include <fable/json/with_boost.hpp>   // for to_json, from_json
#include <fable/schema/ignore.hpp>     // for Ignore
#include <fable/schema/interface.hpp>  // for Base<>, Box

namespace fable {
namespace schema {

/**
 * Passthru stores JSON data and optionally validates it.
 *
 * This is in particular useful when you don't know exactly how the data will
 * be validated, and you want to hold onto the original location metadata for
 * better future error reporting.
 */
template <typename P>
class Passthru : public Base<Passthru<P>> {
 public:  // Types and Constructors
  using Type = Conf;
  using PrototypeSchema = P;

  Passthru(Type* ptr, std::string&& desc)
      : Passthru(ptr, PrototypeSchema(nullptr, ""), std::move(desc)) {}
  Passthru(Type* ptr, const PrototypeSchema& prototype, std::string&& desc)
      : Base<Passthru<P>>(prototype.type(), std::move(desc)), prototype_(prototype), ptr_(ptr) {
    prototype_.reset_ptr();
  }

 public:  // Overrides
  std::string type_string() const override { return prototype_.type_string(); }

  Json json_schema() const override {
    Json j = prototype_.json_schema();
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    // Let the prototype do all the validation, since Passthru doesn't have
    // enough information to do a correct validation.
    prototype_.validate(c);
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = **ptr_;
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = deserialize(c);
  }

  Json serialize(const Type& x) const { return *x; }

  Type deserialize(const Conf& c) const { return c; }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  PrototypeSchema prototype_;
  Type* ptr_{nullptr};
};

inline Passthru<Ignore> make_schema(Conf* ptr, std::string&& desc) {
  return Passthru<Ignore>(ptr, Ignore(), std::move(desc));
}

template <typename P>
Passthru<P> make_schema(Conf* ptr, const P& prototype, std::string&& desc) {
  return Passthru<P>(ptr, prototype, std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_PASSTHRU_HPP_
