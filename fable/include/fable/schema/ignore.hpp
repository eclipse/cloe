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
 * \file fable/schema/ignore.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

/**
 * Ignore always validates true and does not deserialize or serialize types.
 *
 * This is useful when you want to acknowledge the existence of a key in a JSON
 * object (in the Struct schema type) but do not want to serialize or
 * deserialize it. This is much better than telling Struct to allow all
 * additional properties.
 *
 * Specifying the type is optional and serves only for documentation purposes.
 */
class Ignore : public Base<Ignore> {
 public:  // Constructors
  using Type = struct {};

  Ignore() : Base(JsonType::object, "ignored") {}
  explicit Ignore(std::string desc, JsonType t = JsonType::object) : Base(t, std::move(desc)) {}

 public:  // Overrides
  Json json_schema() const override {
    Json j = Json::object({});
    this->augment_schema(j);
    return j;
  }

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override { return true; }
  using Interface::to_json;
  void to_json(Json& j) const override { j = nullptr; }
  void from_conf(const Conf&) override {}
  void reset_ptr() override {}

  Json serialize(const Type&) const { return nullptr; }
  Type deserialize(const Conf&) const { return {}; }
  void serialize_into(Json&, const Type&) const {}
  void deserialize_into(const Conf&, Type&) const {}
};

}  // namespace schema
}  // namespace fable
