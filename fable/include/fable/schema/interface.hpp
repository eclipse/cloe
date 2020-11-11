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
 * \file fable/schema/interface.hpp
 * \see  fable/schema.cpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_INTERFACE_HPP_
#define FABLE_SCHEMA_INTERFACE_HPP_

#include <memory>   // for shared_ptr<>
#include <string>   // for string
#include <utility>  // for move

#include <fable/conf.hpp>   // for Conf
#include <fable/error.hpp>  // for SchemaError
#include <fable/json.hpp>   // for Json

namespace fable {

// Forward declarations:
class Confable;

namespace schema {

/**
 * Interface specifies all information for describing a JSON entity.
 *
 * Schemas are instantiated through one of the many constructors that are
 * defined.
 *
 * ## Notes
 *
 * Most constructor definitions make use of one of a set of private template
 * constructors. These private template constructors enable slimmer
 * constructors for all sorts of types later on. In order to differentiate
 * between these constructors and the ones that users should use, the number of
 * parameters and the order of types is sometimes varied.
 *
 * We could use named constructors, but then you can not use the new operator
 * with the construction or you need to use the copy constructor thereby
 * constructing the schema twice.
 *
 * ## Changes
 *
 * When changing this Interface, the following implementors should be updated:
 *
 *    Box           fable/schema/interface.hpp
 *    Schema        fable/schema.hpp
 *    Variant       fable/schema/variant.hpp
 *    FromConfable  fable/schema/confable.hpp
 *
 */
class Interface {
 public:
  Interface() = default;
  virtual ~Interface() = default;

  /**
   * Return a new instance of the object.
   *
   * This is implemented by Base and allows us to wrap implementors of
   * Interface with Schema.
   */
  virtual Interface* clone() const = 0;

  /**
   * Return whether the accepted input type is a variant.
   *
   * A variant type is one that is composed of other types, such as
   * the special types: any-of, one-of, and all-of.
   */
  virtual bool is_variant() const { return false; }

  /**
   * Return the JSON type.
   *
   * If this is a variant type of differing types, then null should be
   * returned. Otherwise, the type remains unique and can be returned.
   * A type that is null is almost always a variant type of some sort, if
   * even only an optional type.
   */
  virtual JsonType type() const = 0;

  /**
   * Return the type as a string.
   *
   * The format of the string is:
   *
   *    "[array of] TYPE"
   *
   * where TYPE is one of: null, object, boolean, float, integer, unsigned,
   * string, and unknown.
   *
   * Example output:
   *
   *     "object"
   *     "array of boolean"
   *     "array of object"
   */
  virtual std::string type_string() const = 0;

  /**
   * Return whether this interface needs to be set.
   */
  virtual bool is_required() const = 0;

  /**
   * Return human-readable description.
   */
  virtual const std::string& description() const = 0;

  /**
   * Set human-readable description.
   */
  virtual void set_description(const std::string& s) = 0;

  /**
   * Return a compact JSON description of the schema.
   * This is useful for human-readable error output.
   *
   * Example output:
   *
   *    {
   *      "field1": "boolean! :: lorem ipsum dolor sit amet",
   *      "field2": {
   *        "field2.1": "integer :: lorem ipsum dolor sit amet"
   *        "field2.2": "boolean :: lorem ipsum dolor sit amet"
   *      },
   *      "field3": "array of integer :: lorem ipsum dolor sit amet",
   *      "field4": [{
   *        "field4.1": "string :: lorem ipsum dolor sit amet"
   *        "field4.2": "boolean :: lorem ipsum dolor sit amet"
   *      }]
   *    }
   *
   * When describing the schema of a primitive array, we can represent that as
   * a single string. But when the array contains an object, we wrap a single
   * object in an array. This isn't awefully consistent, but it's the best we
   * can do.
   */
  virtual Json usage() const = 0;

  /**
   * Return the JSON schema.
   *
   * Example output:
   *
   *   {
   *     "$schema": "http://json-schema.org/draft-07/schema#",
   *     "description": "stand-in no-operation simulator",
   *     "properties": {
   *       "vehicles": {
   *         "description": "list of vehicle names to make available",
   *         "items": {
   *           "type": "string"
   *         },
   *         "type": "array"
   *       }
   *     },
   *     "title": "nop",
   *     "additionalProperties": false,
   *     "type": "object"
   *   }
   *
   * See the following links for the specification:
   *   - https://json-schema.org
   *   - https://json-schema.org/understanding-json-schema/
   */
  virtual Json json_schema() const = 0;

  /**
   * Validate the input JSON configuration.
   *
   * If you don't have a Conf but would like to validate a Json type,
   * then construct a Conf on the fly:
   *
   *     s.validate(Conf{j});
   *
   * This function is not provided inline to prevent incorrect use.
   */
  virtual void validate(const Conf& c) const = 0;

  /**
   * Return whether the input JSON is valid.
   */
  virtual bool is_valid(const Conf& c) const {
    try {
      validate(c);
    } catch (...) {
      return false;
    }
    return true;
  }

  /**
   * Return the current value of the destination.
   *
   * Warning: This is NOT an efficient operation, but it can be useful for
   * cases where speed is not important.
   */
  virtual void to_json(Json&) const = 0;

  /**
   * Apply the input JSON configuration.
   *
   * This does not validate the input.
   */
  virtual void from_conf(const Conf&) = 0;

  /**
   * Reset the internal pointer to nullptr, protecting against invalid access.
   *
   * This should be used when a schema is used after the backing data has
   * been deleted.
   */
  virtual void reset_ptr() = 0;
};

// ------------------------------------------------------------------------- //

class Box : public Interface {
 public:
  Box() = default;
  Box(const Box&) = default;
  Box(Box&&) = default;
  Box& operator=(const Box&) = default;

  Box(Interface* i) : impl_(i) { assert(impl_); }                             // NOLINT
  Box(std::shared_ptr<Interface> i) : impl_(std::move(i)) { assert(impl_); }  // NOLINT

 public:
  Interface* clone() const override { return impl_->clone(); }
  JsonType type() const override { return impl_->type(); }
  std::string type_string() const override { return impl_->type_string(); }
  bool is_required() const override { return impl_->is_required(); }
  const std::string& description() const override { return impl_->description(); }
  void set_description(const std::string& s) override { return impl_->set_description(s); }
  Json usage() const override { return impl_->usage(); }
  Json json_schema() const override { return impl_->json_schema(); };
  void validate(const Conf& c) const override { impl_->validate(c); }
  void to_json(Json& j) const override { impl_->to_json(j); }
  void from_conf(const Conf& c) override { impl_->from_conf(c); }
  void reset_ptr() override { impl_->reset_ptr(); }

  template <typename T>
  std::shared_ptr<const T> as() const {
    return std::dynamic_pointer_cast<T>(impl_);
  }

  friend void to_json(Json& j, const Box& b) { b.impl_->to_json(j); }

 private:
  std::shared_ptr<Interface> impl_{nullptr};
};

// ------------------------------------------------------------------------- //

template <typename CRTP>
class Base : public Interface {
 public:
  Base() = default;
  Base(JsonType t, std::string&& desc) : type_(t), desc_(std::move(desc)) {}
  explicit Base(JsonType t) : type_(t) {}
  explicit Base(std::string&& desc) : desc_(std::move(desc)) {}
  virtual ~Base() = default;

  Interface* clone() const override { return new CRTP(static_cast<CRTP const&>(*this)); }
  operator Box() const { return Box{this->clone()}; }

  JsonType type() const override { return type_; }
  std::string type_string() const override { return to_string(type_); }

  Json usage() const override {
    auto required = required_ ? "!" : "";
    if (desc_.empty()) {
      return type_string() + required;
    } else {
      return fmt::format("{}{} :: {}", type_string(), required, desc_);
    }
  }

  bool is_required() const override { return required_; }
  CRTP require() && {
    required_ = true;
    return std::move(*dynamic_cast<CRTP*>(this));
  }
  CRTP required(bool value) && {
    required_ = value;
    return std::move(*dynamic_cast<CRTP*>(this));
  }

  CRTP reset_pointer() && {
    reset_ptr();
    return std::move(*dynamic_cast<CRTP*>(this));
  }

  bool has_description() const { return !desc_.empty(); }
  void set_description(const std::string& s) override { desc_ = s; }
  void set_description(std::string&& s) { desc_ = std::move(s); }
  const std::string& description() const override { return desc_; }
  CRTP description(std::string&& desc) && {
    desc_ = std::move(desc);
    return std::move(*dynamic_cast<CRTP*>(this));
  }

 protected:
  void validate_type(const Conf& c) const {
    if (c->type() != type_) {
      if (c->type() == JsonType::number_unsigned && type_ == JsonType::number_integer) {
        return;
      }

      throw SchemaError{c, this->json_schema(), "require type {}, got {}", type_string(),
                        to_string(c->type())};
    }
  }

  template <typename... Args>
  [[noreturn]] void throw_error(const Conf& c, const char* format, Args... args) const {
    throw SchemaError{c, this->json_schema(), format, args...};
  }

  [[noreturn]] void throw_error(const ConfError& e) const {
    throw SchemaError{e, this->json_schema()};
  }

  [[noreturn]] void throw_wrong_type(const Conf& c) const {
    throw_error(error::WrongType(c, type_));
  }

  void augment_schema(Json& j) const {
    if (!desc_.empty()) {
      j["description"] = desc_;
    }
  }

 protected:
  JsonType type_{JsonType::null};
  bool required_{false};
  std::string desc_{};
};

// ------------------------------------------------------------------------- //

template <typename T, std::enable_if_t<std::is_base_of<Confable, T>::value, int> = 0>
auto make_prototype(std::string&& desc = "");

template <typename T, std::enable_if_t<!std::is_base_of<Confable, T>::value, int> = 0>
auto make_prototype(std::string&& desc = "");

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_INTERFACE_HPP_
