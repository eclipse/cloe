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
 * \file fable/schema/variant.hpp
 * \see  fable/schema/variant.cpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <fable/schema/interface.hpp>  // for Interface

namespace fable::schema {

using BoxVec = std::vector<Box>;
using BoxList = std::initializer_list<Box>;

/**
 * Variant deserializes JSON data into one of multiple variants.
 *
 * Whenever the JSON input can come in different forms but is more or less
 * destined for the same variable (or set of variables), we need to be able to
 * deal with variants. For example, an enumeration is a variant of constants.
 *
 * Deciding which variant to use for deserializing JSON is not always
 * unambiguous. This class will simply use the first valid schema. For this
 * reason it is strongly recommended to pay attention to making all schemas
 * that are part of a variant distinct from one another. One way to do this
 * is to introduce a required constant (such as a value from an enumeration)
 * into the schema.
 *
 * Serialization uses the very first schema in the variant list for output.
 * For this reason, the very first schema in the list should contain the
 * schema of the desired output.
 */
class Variant : public Interface {
 public:  // Constructors
  Variant(std::initializer_list<Box> vec) : Variant("", vec) {}
  Variant(std::string desc, std::initializer_list<Box> vec)
      : Variant(std::move(desc), std::vector<Box>(vec)) {}

  Variant(std::vector<Box>&& vec) : Variant("", std::move(vec)) {}  // NOLINT(runtime/explicit)
  Variant(std::string desc, std::vector<Box>&& vec);

 public:  // Base
  [[nodiscard]] std::unique_ptr<Interface> clone() const override {
    return std::make_unique<Variant>(*this);
  }
  [[nodiscard]] operator Box() const { return Box{this->clone()}; }
  [[nodiscard]] JsonType type() const override { return type_; }
  [[nodiscard]] std::string type_string() const override { return type_string_; }
  [[nodiscard]] bool is_variant() const override { return true; }
  [[nodiscard]] Json usage() const override;

  [[nodiscard]] bool is_required() const override { return required_; }
  [[nodiscard]] Variant require() && {
    required_ = true;
    return std::move(*this);
  }
  void set_required(bool value) {
    required_ = value;
  }
  [[nodiscard]] Variant required(bool value) && {
    required_ = value;
    return std::move(*this);
  }

  [[nodiscard]] bool has_description() const { return !desc_.empty(); }
  void set_description(std::string s) override { desc_ = std::move(s); }
  [[nodiscard]] const std::string& description() const override { return desc_; }
  [[nodiscard]] Variant description(std::string desc) && {
    desc_ = std::move(desc);
    return std::move(*this);
  }

 public:  // Special
  [[nodiscard]] bool unique_match() const {
    return unique_match_;
  }
  void set_unique_match(bool value) {
    unique_match_ = value;
  }
  [[nodiscard]] Variant unique_match(bool value) && {
    unique_match_ = value;
    return std::move(*this);
  }

  [[nodiscard]] Variant reset_pointer() && {
    reset_ptr();
    return std::move(*this);
  }

 public:  // Overrides
  using Interface::to_json;
  [[nodiscard]] Json json_schema() const override;
  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    return validate_index(c, err).has_value();
  }
  void to_json(Json& j) const override { schemas_[0].to_json(j); }
  void from_conf(const Conf& c) override {
    auto index = variant_index(c);
    schemas_[index].from_conf(c);
  }

  // TODO: Implement or explain why we don't need the following methods:
  // - serialize
  // - serialize_into
  // - deserialize
  // - deserialize_into

  void reset_ptr() override;

 private:
  std::optional<size_t> validate_index(const Conf& c, std::optional<SchemaError>& err) const;
  [[nodiscard]] size_t variant_index(const Conf& c) const;

 private:
  std::string desc_{};
  std::vector<Box> schemas_;
  bool required_{false};
  JsonType type_{JsonType::null};
  std::string type_string_{};
  bool unique_match_{false};
};

}  // namespace fable::schema
