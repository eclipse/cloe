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
 * \file fable/schema.hpp
 * \see  fable/schema.cpp
 * \see  fable/schema_test.cpp
 * \see  fable/confable.hpp
 *
 * This file includes most types from the schema namespace and defines the
 * Schema type.
 *
 * Schema is a class that can be used to describe how to set the values of
 * a C++ datatype from a Json. This description can be used to
 *
 *  - output a JSON Schema,
 *  - validate a JSON input,
 *  - deserialize a JSON input,
 *  - serialize a C++ type to JSON.
 *
 * The last activity is not necessarily the most effective way to serialize
 * a datatype, as there is quite a lot of indirection involved. However, Schema
 * is designed to be *the* way to deserialize JSON. It is recommended to do
 * this by making your type derive from Confable. Part of the Confable
 * interface is to provide a Schema, which is then used for validation and
 * deserialization.
 *
 * There are several ways to define a Schema.
 *
 * 1. Use Schema interface.
 *    The Schema class provides several constructors that take various
 *    datatypes and instantiate a correct underlying type. The advantage is
 *    that it looks good and doesn't require you to know anything about the
 *    underlying types. The great disadvantage is that the type-specific
 *    methods can't be used to further refine validation.
 *
 * 2. Use make_schema functions.
 *    These functions can be used to return the underlying types. The advantage
 *    is that it looks okay and doesn't require you to know anything about the
 *    underlying types. It also returns the underlying types, which means you
 *    can use type-specific methods to further refine validation. This is the
 *    recommended way to create Schemas. These functions are defined in headers
 *    that can be found in the schema/ directory, but have in principle exactly
 *    the same syntax as the Schema interface. Thus it is generally possible to
 *    simply replace all instances of Schema with make_schema.
 *
 * 3. Use types in schema namespace.
 *    This is kind of ugly but provides you with the most control. Because the
 *    types are templated, you often have to be very specific when
 *    instantiating a type. These types are defined in headers that can be
 *    found in the schema/ directory.
 *
 * Examples
 * --------
 *
 * Given the following type:
 *
 *    struct MyData {
 *      std::string host{"localhost"};
 *      uint16_t port{0};
 *
 *      Schema schema_impl();
 *    };
 *
 * We can implement the schema_impl method in the three ways detailed above.
 *
 * 1. Using Schema interface:
 *
 *      Schema MyData::schema_impl() {
 *        return Schema{
 *          {"host", Schema(&host, "hostname of connection")},
 *          {"port", Schema(&port, "port of connection")},
 *        };
 *      }
 *
 * 2. Using make_schema functions:
 *
 *      Schema MyData::schema_impl() {
 *        return make_schema({
 *          {"host", make_schema(&host, "hostname of connection").require()},
 *          {"port", make_schema(&port, "port of connection").minimum(1024)},
 *        });
 *      }
 *
 * 3. Using types in schema namespace:
 *
 *      Schema MyData::schema_impl() {
 *        using namepsace schema;
 *        return Struct{
 *          {"host", String(&host, "hostname of connection").require()},
 *          {"port", Number<uint16_t>(&port, "port of connection").minimum(1024)},
 *        };
 *      }
 *
 * Note that Schema contains pointers to the variables of MyData. The schema
 * is therefore invalidated whenever MyData is moved or copied. The Confable
 * type takes care of these issues to ensure that you don't shoot yourself in
 * the foot.
 */

#pragma once
#ifndef FABLE_SCHEMA_HPP_
#define FABLE_SCHEMA_HPP_

#include <chrono>       // for duration<>
#include <map>          // for map<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_arithmetic<>, is_enum<>, ...
#include <utility>      // for move
#include <vector>       // for vector<>

#include <boost/optional.hpp>  // for optional<>

#include <fable/schema/array.hpp>      // for Array<>
#include <fable/schema/boolean.hpp>    // for Boolean
#include <fable/schema/confable.hpp>   // for FromConfable
#include <fable/schema/const.hpp>      // for Const<>
#include <fable/schema/duration.hpp>   // for Duration<>
#include <fable/schema/enum.hpp>       // for Enum<>
#include <fable/schema/ignore.hpp>     // for Ignore
#include <fable/schema/interface.hpp>  // for Interface, Box
#include <fable/schema/json.hpp>       // for FromJson<>
#include <fable/schema/map.hpp>        // for Map<>
#include <fable/schema/number.hpp>     // for Number<>
#include <fable/schema/optional.hpp>   // for Optional<>
#include <fable/schema/passthru.hpp>   // for Passthru
#include <fable/schema/path.hpp>       // for Path
#include <fable/schema/string.hpp>     // for String
#include <fable/schema/struct.hpp>     // for Struct
#include <fable/schema/variant.hpp>    // for Variant

// It is important that this include comes after all the other ones,
// so that it has access to ALL the previous definitions.
#include <fable/schema/magic.hpp>  // for make_prototype, ...

namespace fable {

// Bring all make_* functions into the fable namespace.
using schema::make_const_schema;
using schema::make_const_str;
using schema::make_prototype;
using schema::make_schema;

/**
 * Define the automatically deduced schema class of a given type.
 *
 * \example
 *
 *     using VecSchema = schema_type<std::vector<int64_t>>::type;
 */
template <typename T>
struct schema_type {
  using type = decltype(make_schema(static_cast<T*>(nullptr), ""));
};

/**
 * Schema is a wrapper class for fable schemas that automatically
 * chooses the correct underlying schema type.
 *
 * That is, this class provides an interface that doesn't require you to know
 * which class to use, but it doesn't cover all use-cases.
 */
class Schema : public schema::Interface {
 public:
  // Operators
  Schema(const Schema&) = default;
  Schema(Schema&&) = default;
  Schema& operator=(const Schema&) = default;

  // Struct
  Schema(std::string&& desc, schema::PropertyList<> props)
      : impl_(new schema::Struct(std::move(desc), props)) {}

  Schema(schema::PropertyList<> props) : Schema("", props) {}

  Schema(std::string&& desc, const Schema& base, schema::PropertyList<> props)
      : impl_(new schema::Struct(std::move(desc), base, props)) {}

  Schema(const Schema& base, schema::PropertyList<> props) : Schema("", base, props) {}

  // Variant
  Schema(const std::vector<Schema>& xs);  // NOLINT(runtime/explicit)
  Schema(std::string&& desc, const std::vector<Schema>& xs);

  Schema(schema::BoxList props);  // NOLINT(runtime/explicit)
  Schema(std::string&& desc, schema::BoxList props);

  Schema(schema::BoxVec&& props);  // NOLINT(runtime/explicit)
  Schema(std::string&& desc, schema::BoxVec&& props);

  // Interface
  template <typename T, std::enable_if_t<std::is_base_of<schema::Interface, T>::value, int> = 0>
  Schema(const T& value) : impl_(value.clone()) {}                      // NOLINT(runtime/explicit)
  Schema(schema::Interface* i) : impl_(i) { assert(impl_); }            // NOLINT(runtime/explicit)
  Schema(std::shared_ptr<schema::Interface> i) : impl_(std::move(i)) {  // NOLINT(runtime/explicit)
    assert(impl_);
  }

  // Ignore
  Schema() : impl_(new schema::Ignore("")) {}
  explicit Schema(std::string&& desc, JsonType t = JsonType::object)
      : impl_(new schema::Ignore(std::move(desc), t)) {}

  // Primitives
  template <typename T>
  Schema(T* ptr, std::string&& desc) : impl_(make_schema(ptr, std::move(desc)).clone()) {}
  template <typename T>
  Schema(T* ptr, const schema::Box& prototype, std::string&& desc)
      : impl_(make_schema(ptr, prototype, std::move(desc)).clone()) {}

  // FromJson
  template <typename T>
  Schema(T* ptr, JsonType t, std::string&& desc)
      : impl_(new schema::FromJson<T>(ptr, t, std::move(desc))) {}

 public:  // Special
  Schema reset_pointer() && {
    reset_ptr();
    return std::move(*this);
  }

  Json json_schema_qualified() const {
    Json j = impl_->json_schema();
    j["$schema"] = "http://json-schema.org/draft-07/schema#";
    return j;
  }

  Json json_schema_qualified(const std::string& id) const {
    Json j = json_schema_qualified();
    j["$id"] = id;
    return j;
  }

  Json to_json() const {
    Json j;
    to_json(j);
    return j;
  }

  friend void to_json(Json& j, const Schema& s) { s.impl_->to_json(j); }

  template <typename T>
  std::shared_ptr<const T> as() const {
    return std::dynamic_pointer_cast<T>(impl_);
  }

 public:  // Overrides
  operator schema::Box() const { return schema::Box{impl_}; }
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

 private:
  std::shared_ptr<schema::Interface> impl_{nullptr};
};

}  // namespace fable

#endif  // FABLE_SCHEMA_HPP_
