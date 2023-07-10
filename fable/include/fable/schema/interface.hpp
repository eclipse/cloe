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

#include <memory>       // for shared_ptr<>
#include <optional>     // for optional<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_base_of<>
#include <utility>      // for move

#include <fable/conf.hpp>       // for Conf
#include <fable/error.hpp>      // for SchemaError
#include <fable/fable_fwd.hpp>  // for Confable
#include <fable/json.hpp>       // for Json

namespace fable {
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
  virtual void set_description(std::string s) = 0;

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
   * object in an array. This isn't awfully consistent, but it's the best we
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
   * Validate the input JSON configuration for correctness.
   *
   * - This method should only set `error` if there is an error.
   *   This method should not reset `error` if there is no error.
   *   Therefore, the content of `error` is only valid if the method returns false.
   *   This allows you to chain validates and check at the end if there was an error.
   * - This method should not throw if there is a schema error!
   *
   * \param c JSON to check
   * \param error reference to store error if occurred
   * \return true if valid
   */
  virtual bool validate(const Conf& c, std::optional<SchemaError>& error) const = 0;

  /**
   * Validate the input JSON configuration or throw an error.
   *
   * If you don't have a Conf but would like to validate a Json type,
   * then construct a Conf on the fly:
   *
   *     s.validate_or_throw(Conf{j});
   *
   * This overload is not provided inline to prevent incorrect use.
   *
   * \param c JSON to check
   */
  virtual void validate_or_throw(const Conf& c) const final {
    if (auto err = fail(c); err) {
      throw std::move(*err);
    }
  }

  /**
   * Return input JSON configuration schema error, if any.
   *
   * If you don't have a Conf but would like to validate a Json type,
   * then construct a Conf on the fly:
   *
   *     s.fail(Conf{j});
   *
   * This overload is not provided inline to prevent incorrect use.
   *
   * \param c JSON to check
   * \return error if invalid
   */
  [[nodiscard]] virtual std::optional<SchemaError> fail(const Conf& c) const final {
    std::optional<SchemaError> err;
    validate(c, err);
    return err;
  }

  /**
   * Return whether the input JSON is valid.
   */
  [[nodiscard]] virtual bool is_valid(const Conf& c) const final { return !fail(c); }

  /**
   * Return the current value of the destination.
   *
   * Warning: This is NOT an efficient operation, but it can be useful for
   * cases where speed is not important.
   */
  virtual Json to_json() const {
    Json j;
    to_json(j);
    return j;
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

/**
 * Use SFINAE mechanism to disable a template function when S is not a subclass
 * of Interface, hence not a schema.
 *
 * \example
 *     template<typename S, typename = enable_if_schema_t<S>>
 *     void foobar(S schema);
 */
template <typename S>
using enable_if_schema_t = std::enable_if_t<std::is_base_of_v<Interface, S>>;

// ------------------------------------------------------------------------- //

class Box : public Interface {
 public:  // Constructors
  Box() = default;
  Box(const Box&) = default;
  Box(Box&&) = default;
  Box& operator=(const Box&) = default;

  Box(Interface* i) : impl_(i) { assert(impl_); }                             // NOLINT
  Box(std::shared_ptr<Interface> i) : impl_(std::move(i)) { assert(impl_); }  // NOLINT

 public:  // Special
  /**
   * Return the underlying Interface.
   */
  std::shared_ptr<Interface> get() { return impl_; }

  /**
   * Return this type as a pointer to T.
   *
   * This method can be used like so:
   *
   *     // In this example, we want a Struct.
   *     auto ptr = s.template as<Struct>();
   *
   */
  template <typename T>
  std::shared_ptr<T> as() const {
    assert(impl_ != nullptr);
    auto downcast_ptr = std::dynamic_pointer_cast<T>(impl_);
    if (downcast_ptr == nullptr) {
      // If you can't figure out the type of this even from this error, set
      // a breakpoint here in gdb and run:
      //
      //     p dynamic_cast<Interface*>(impl_.get())
      //
      // That will give you something like:
      //
      //     $8 = (fable::schema::Interface *) 0x5555559dfd30 <
      //          vtable for fable::schema::FromConfable<
      //            (anonymous namespace)::NormalDistribution<double>, 0
      //          >
      //          +16>
      //
      // Which is much more specific, since the output of FromConfable and
      // Struct can be equivalent, for example.
      throw SchemaError{Conf{}, this->json_schema(),
                        "cannot dynamic_pointer_cast to type T, got nullptr"};
    }
    return downcast_ptr;
  }

  /**
   * Return this type as a pointer to T.
   *
   * This is unsafe in that you need to check the resulting shared pointer
   * yourself for whether the dynamic cast was successful or not.
   */
  template <typename T>
  std::shared_ptr<T> as_unsafe() const {
    return std::dynamic_pointer_cast<T>(impl_);
  }

  Box reset_pointer() && {
    reset_ptr();
    return std::move(*this);
  }

 public:  // Overrides
  using Interface::to_json;
  Interface* clone() const override { return impl_->clone(); }
  JsonType type() const override { return impl_->type(); }
  std::string type_string() const override { return impl_->type_string(); }
  bool is_required() const override { return impl_->is_required(); }
  const std::string& description() const override { return impl_->description(); }
  void set_description(std::string s) override { return impl_->set_description(std::move(s)); }
  Json usage() const override { return impl_->usage(); }
  Json json_schema() const override { return impl_->json_schema(); };
  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    return impl_->validate(c, err);
  }
  void to_json(Json& j) const override { impl_->to_json(j); }
  void from_conf(const Conf& c) override { impl_->from_conf(c); }
  void reset_ptr() override { impl_->reset_ptr(); }

  friend void to_json(Json& j, const Box& b) { b.impl_->to_json(j); }

 private:
  std::shared_ptr<Interface> impl_{nullptr};
};

// ------------------------------------------------------------------------- //

/**
 * The Base class implements the Interface partially and is meant to cover
 * the most commonly used types.
 *
 * See most of the other schema types for how it is used.
 */
template <typename CRTP>
class Base : public Interface {
 public:
  Base() = default;
  Base(JsonType t, std::string desc) : type_(t), desc_(std::move(desc)) {}
  explicit Base(JsonType t) : type_(t) {}
  explicit Base(std::string desc) : desc_(std::move(desc)) {}
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
  void set_description(std::string s) override { desc_ = std::move(s); }
  const std::string& description() const override { return desc_; }
  CRTP description(std::string desc) && {
    desc_ = std::move(desc);
    return std::move(*dynamic_cast<CRTP*>(this));
  }

 protected:
  /**
   * Validate whether `c` is of the correct type.
   *
   * This method is provided for an implementation to call in its `fail()`
   * implementation. It is not called automatically.
   */
  bool validate_type(const Conf& c, std::optional<SchemaError>& err) const {
    if (c->type() != type_) {
      if (c->type() == JsonType::number_unsigned && type_ == JsonType::number_integer) {
        return true;
      }

      return this->set_error(err, c, "require type {}, got {}", type_string(), to_string(c->type()));
    }
    return true;
  }

  template <typename... Args>
  [[nodiscard]] SchemaError error(const Conf& c, std::string_view format, Args&&... args) const {
    return SchemaError{c, this->json_schema(), format, std::forward<Args>(args)...};
  }

  [[nodiscard]] SchemaError error(const ConfError& e) const {
    return SchemaError{e, this->json_schema()};
  }

  [[nodiscard]] SchemaError wrong_type(const Conf& c) const {
    return error(error::WrongType(c, type_));
  }

  template <typename... Args>
  bool set_error(std::optional<SchemaError>& err, const Conf& c, std::string_view format, Args&&... args) const {
    err.emplace(this->error(c, format, std::forward<Args>(args)...));
    return false;
  }

  bool set_error(std::optional<SchemaError>& err, const ConfError& e) const {
    err.emplace(this->error(e));
    return false;
  }

  bool set_error(std::optional<SchemaError>& err, SchemaError&& e) const {
    err.emplace(std::move(e));
    return false;
  }

  bool set_wrong_type(std::optional<SchemaError>& err, const Conf& c) const {
    err.emplace(this->wrong_type(c));
    return false;
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

/**
 * Use SFINAE mechanism to disable a template function when S is not a subclass
 * of Confable.
 *
 * \example
 *     template<typename T, typename = enable_if_confable_t<T>>
 *     void foobar(T x);
 */
template <typename T>
using enable_if_confable_t = std::enable_if_t<std::is_base_of_v<Confable, T>>;

/**
 * Use SFINAE mechanism to disable a template function when S is a subclass
 * of Confable.
 *
 * \example
 *     template<typename T, typename = enable_if_not_confable_t<T>>
 *     void foobar(T x);
 */
template <typename T>
using enable_if_not_confable_t = std::enable_if_t<!std::is_base_of_v<Confable, T>>;

template <typename T, std::enable_if_t<std::is_base_of_v<Confable, T>, int> = 0>
auto make_prototype(std::string desc = "");

template <typename T, std::enable_if_t<!std::is_base_of_v<Confable, T>, int> = 0>
auto make_prototype(std::string desc = "");

}  // namespace schema
}  // namespace fable
