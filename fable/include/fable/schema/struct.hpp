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
 * \file fable/schema/struct.hpp
 * \see  fable/schema/struct.cpp
 * \see  fable/schema_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_STRUCT_HPP_
#define FABLE_SCHEMA_STRUCT_HPP_

#include <map>          // for map<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_same<>
#include <utility>      // for move, forward<>, pair<>
#include <vector>       // for vector<>

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

/**
 * PropertyList is mainly used in constructors to enable the use of initializer
 * list.
 *
 * \example
 *   Given the following constructor declaration:
 *
 *       Struct(PropertyList<> props);
 *
 *   Struct can be instantiated then like so:
 *
 *     return Struct{
 *        {"prop_a", make_schema(...)},
 *        {"prop_b", make_schema(...)},
 *     };
 */
template <typename S = Box>
using PropertyList = std::initializer_list<std::pair<std::string const, S>>;

template <typename T, typename S = Box>
using enable_if_property_list_t = std::enable_if_t<std::is_same<PropertyList<S>, T>::value>;

/**
 * Struct maintains a key-value mapping, where the list of keys is usually
 * known and the values can have different types (schemas).
 *
 * This is usually the basis of any schema, since most start with a JSON
 * object.
 *
 * This should not be confused with the Map type.
 *
 * \see fable/schema/map.hpp
 */
class Struct : public Base<Struct> {
 public:  // Constructors
  explicit Struct(std::string&& desc = "") : Base(JsonType::object, std::move(desc)) {}

  Struct(std::string&& desc, PropertyList<Box> props) : Base(JsonType::object, std::move(desc)) {
    set_properties(props);
  }

  Struct(PropertyList<Box> props) : Struct("", props) {}  // NOLINT

  // Inheriting constructors
  /**
   * Instantiate a Struct with a base Schema, which should also be a Struct,
   * then extend it with the property list.
   *
   * This is particularly useful when the Confable is inheriting from a base
   * class:
   *
   *     struct Sub : public Base {
   *       std::string member;
   *       CONFABLE_SCHEMA(Sub) {
   *         return Struct{
   *           Base::schema_impl(),
   *           {
   *             {"member", make_schema(&member, "important addition")},
   *           }
   *         };
   *       }
   *     };
   *
   * Warning: When implementing schema_impl, for example with CONFABLE_SCHEMA,
   * it is absolutely important that you _not_ call Base::schema(), as this
   * will internally call this->schema_impl(), which will lead to an
   * infinite recursion! Instead, call Base::schema_impl().
   */
  Struct(std::string&& desc, const Box& base, PropertyList<Box> props)
      : Struct(*base.template as<Struct>()) {
    desc_ = std::move(desc);
    set_properties(props);
  }

  Struct(const Box& base, PropertyList<Box> props) : Struct("", base, props) {}

 public:  // Special
  /**
   * Set the property to this schema.
   *
   * - This overwrites any already existing field of the same key.
   */
  void set_property(const std::string& key, Box&& s);

  Struct property(const std::string& key, Box&& s) && {
    set_property(key, std::move(s));
    return std::move(*this);
  }

  /**
   * Set all properties.
   *
   * - This overwrites any already existing property of the same key.
   */
  void set_properties(PropertyList<Box> props);

  /**
   * Add the properties from s to this schema.
   *
   * - This will overwrite any existing properties.
   */
  void set_properties_from(const Struct& s) { set_properties(s.properties_); }

  void set_properties_from(const Box& s) { set_properties_from(*s.template as<Struct>()); }

  template <typename T, typename = enable_if_confable_t<T>>
  void set_properties_from(const T* x) {
    set_properties_from(x->schema());
  }

  template <typename T>
  Struct properties_from(const T x) {
    set_properties_from(x);
    return std::move(*this);
  }

  /**
   * Set which entries are required.
   *
   * - Complexity: O(n*m) with n the number of current properties and
   *   m the number of properties in init.
   */
  void set_require(std::initializer_list<std::string> init);
  Struct require(std::initializer_list<std::string> init) &&;

  /**
   * Set whether all entries are required.
   *
   * - The default is false.
   * - Meant to be used during construction.
   * - This will only act on properties that exist at the time that this is
   *   called.
   */
  void set_require_all();
  Struct require_all() &&;

  /**
   * Set whether to tolerate unknown fields in this entry.
   *
   * - The default is false.
   * - Meant to be used during construction.
   */
  Struct additional_properties(bool v) && {
    additional_properties_ = v;
    return std::move(*this);
  }

  template <typename S, typename = enable_if_schema_t<S>>
  void set_additional_properties(const S& s) {
    additional_properties_ = true;
    additional_prototype_.reset(s.clone());
    additional_prototype_->reset_ptr();
  }

  template <typename S, typename = enable_if_schema_t<S>>
  Struct additional_properties(const S& s) && {
    set_additional_properties(s);
    return std::move(*this);
  }

  bool additional_properties() const { return additional_properties_; }

  void reset_ptr() override;

 public:  // Overrides
  using Interface::to_json;
  Json usage() const override;
  Json json_schema() const override;
  void validate(const Conf& c) const override;
  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override;

 private:
  void set_properties(const std::map<std::string, Box>& props);

 private:
  std::map<std::string, Box> properties_{};
  std::vector<std::string> properties_required_{};
  std::shared_ptr<Interface> additional_prototype_{};
  bool additional_properties_{false};
};

template <typename T, typename = enable_if_property_list_t<T>>
inline Struct make_schema(T&& props) {
  return Struct(std::forward<T>(props));
}

template <typename T, typename = enable_if_property_list_t<T>>
inline Struct make_schema(std::string&& desc, T&& props) {
  return Struct(std::move(desc), std::forward<T>(props));
}

template <typename T, typename = enable_if_property_list_t<T>>
inline Struct make_schema(std::string&& desc, const Box& base, T&& props) {
  return Struct(std::move(desc), base, std::forward<T>(props));
}

template <typename T, typename = enable_if_property_list_t<T>>
inline Struct make_schema(std::string&& desc, const Struct& base, T&& props) {
  return Struct(std::move(desc), base, std::forward<T>(props));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_STRUCT_HPP_
