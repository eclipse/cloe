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
 * \file fable/schema/factory.hpp
 * \see  fable/schema/magic.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_FACTORY_HPP_
#define FABLE_SCHEMA_FACTORY_HPP_

#include <limits>   // for numeric_limits<>
#include <map>      // for map<>
#include <memory>   // for shared_ptr<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <fable/schema/const.hpp>      // for Const
#include <fable/schema/interface.hpp>  // for Base<>, Box
#include <fable/schema/string.hpp>     // for String
#include <fable/schema/struct.hpp>     // for Struct
#include <fable/schema/variant.hpp>    // for Variant

namespace fable {
namespace schema {

/**
 * Factory is a schema of factories.
 *
 * F is a class with a clone() method and T is a Confable with a constructor
 * with the signature:
 *
 *    T(const std::string&, std::shared_ptr<F>)
 *
 * This schema cannot serialize or deserialize into pointers, it can only
 * create new instances with the `make` method.
 */
template <typename T, typename F>
class Factory : public Base<Factory<T, F>> {
  std::string BINDING_KEYWORD{"binding"};

 public:  // Types and Constructors
  using Type = T;

  explicit Factory(std::string&& desc = "")
      : Base<Factory<T, F>>(JsonType::object, std::move(desc)) {}

 public:  // Special
  const std::map<std::string, std::shared_ptr<F>> factories() const { return available_; }

  std::shared_ptr<F> get_factory(const std::string& key) const { return available_.at(key); }

  bool has_factory(const std::string& key) const { return available_.count(key); }

  void add_factory(const std::string& key, std::shared_ptr<F> f) {
    available_.insert(std::make_pair(key, f));
    schema_.reset(new Variant(factory_schemas()));
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j;
    if (available_.empty()) {
      j["not"] = Json{
          {"description", "no variants available"},
      };
    } else {
      j["oneOf"] = factory_json_schemas();
    }
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    assert(schema_ != nullptr);
    auto binding = c.get<std::string>(BINDING_KEYWORD);
    if (!available_.count(binding)) {
      this->throw_error(c, "unknown binding: {}", binding);
    }
    schema_->validate(c);
  }

  Type make(const Conf& c) const { return deserialize(c); }

  Type deserialize(const Conf& c) const {
    assert(schema_ != nullptr);
    auto binding = c.get<std::string>(BINDING_KEYWORD);
    const auto& f = available_.at(binding);
    T tmp{binding, f->clone()};
    tmp.from_conf(c);
    return tmp;
  }

  Json serialize(const Type& x) const { return x; }

  void from_conf(const Conf&) override {
    throw std::logic_error("invalid call: Factory cannot modify existing object");
  }

  void to_json(Json&) const override {
    throw std::logic_error("invalid call: Factory does not have any state");
  }

  void reset_ptr() override {
    // Factory<T, F> only puts Schemas in schema_ that have already had their
    // pointer reset to nullptr (see factory_schemas()).
    // Therefore there is nothing to do here.
  }

 protected:
  std::vector<fable::schema::Box> factory_schemas() const {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    std::vector<Box> out;
    out.reserve(available_.size());
    for (auto& kv : available_) {
      // clang-format off
      out.emplace_back(
        Struct{
          {BINDING_KEYWORD, make_const_str(kv.first, "name of simulator binding").require()},
          {"name", String{nullptr, "identifier of simulator instance"}},
          {"args", kv.second->schema()},
        }.additional_properties(true).reset_pointer()
      );
      // clang-format on
    }
    return out;
  }

  std::vector<Json> factory_json_schemas() const {
    auto schemas = factory_schemas();
    std::vector<Json> out;
    out.reserve(schemas.size());
    for (auto& s : schemas) {
      out.emplace_back(s.json_schema());
    }
    return out;
  }

 private:
  std::shared_ptr<Variant> schema_;
  std::map<std::string, std::shared_ptr<F>> available_;
};

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_FACTORY_HPP_
