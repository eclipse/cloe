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

using BoxPairList = std::initializer_list<std::pair<std::string const, Box>>;
using BoxMap = std::map<std::string, Box>;

template <typename T>
using is_properties_t =
    std::enable_if_t<std::is_same<BoxPairList, T>::value || std::is_same<BoxMap, T>::value, int>;

/**
 * Struct maintains a key-value mapping, where the list of keys is usually
 * known and the values can have different types (schemas).
 *
 * This is usually the basis of any schema, since most start with a JSON
 * object.
 *
 * This should not be confused with the Map type.
 *
 * \see  fable/schema/map.hpp
 */
class Struct : public Base<Struct> {
 public:  // Constructors
  explicit Struct(std::string&& desc = "") : Base(JsonType::object, std::move(desc)) {}

  Struct(std::string&& desc, BoxPairList props);
  Struct(std::string&& desc, BoxMap&& props);
  Struct(std::string&& desc, const Struct& base, BoxPairList props);
  Struct(std::string&& desc, const Struct& base, BoxMap&& props);
  Struct(std::string&& desc, const Box& base, BoxPairList props);
  Struct(std::string&& desc, const Box& base, BoxMap&& props);

  Struct(BoxPairList props) : Struct("", std::move(props)) {}  // NOLINT
  Struct(BoxMap&& props) : Struct("", std::move(props)) {}     // NOLINT
  Struct(const Struct& base, BoxPairList props) : Struct("", base, std::move(props)) {}
  Struct(const Struct& base, BoxMap&& props) : Struct("", base, std::move(props)) {}
  Struct(const Box& base, BoxPairList props) : Struct("", base, std::move(props)) {}
  Struct(const Box& base, BoxMap&& props) : Struct("", base, std::move(props)) {}

 public:  // Special
  /**
   * Set the property to this schema.
   *
   * - This overwrites any already existing field of the same key.
   * - Meant to be used during construction.
   */
  void set_property(const std::string& key, Box&& s);
  Struct property(const std::string& key, Box&& s) &&;

  /**
   * Add the properties from s to this schema.
   *
   * - This will overwrite any existing properties.
   */
  void set_properties_from(const Struct& s);
  Struct properties_from(const Struct& s) &&;

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

  // XXX(ben): This does not work!
  template <typename T, std::enable_if_t<std::is_base_of<Interface, T>::value, int> = 0>
  void set_additional_properties(const T& s) {
    additional_properties_ = true;
    additional_prototype_.reset(s.clone());
    additional_prototype_->reset_ptr();
  }

  // XXX(ben): This does not work!
  template <typename T, std::enable_if_t<std::is_base_of<Interface, T>::value, int> = 0>
  Struct additional_properties(const T& s) && {
    set_additional_properties(s);
    return std::move(*this);
  }

  bool additional_properties() const { return additional_properties_; }

  void reset_ptr() override;

 public:  // Overrides
  Json usage() const override;
  Json json_schema() const override;
  void validate(const Conf& c) const override;
  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override;

 private:
  std::map<std::string, Box> properties_{};
  std::vector<std::string> properties_required_{};
  std::shared_ptr<Interface> additional_prototype_{};
  bool additional_properties_{false};
};

template <typename T, is_properties_t<T> = 0>
inline Struct make_schema(T&& props) {
  return Struct(std::forward<T>(props));
}

template <typename T, is_properties_t<T> = 0>
inline Struct make_schema(std::string&& desc, T&& props) {
  return Struct(std::move(desc), std::forward<T>(props));
}

template <typename T, is_properties_t<T> = 0>
inline Struct make_schema(std::string&& desc, const Box& base, T&& props) {
  return Struct(std::move(desc), base, std::forward<T>(props));
}

template <typename T, is_properties_t<T> = 0>
inline Struct make_schema(std::string&& desc, const Struct& base, T&& props) {
  return Struct(std::move(desc), base, std::forward<T>(props));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_STRUCT_HPP_
