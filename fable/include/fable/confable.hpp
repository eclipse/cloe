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
 * \file fable/confable.hpp
 * \see  fable/confable.cpp
 * \see  fable/schema.hpp
 */

#pragma once
#ifndef FABLE_CONFABLE_HPP_
#define FABLE_CONFABLE_HPP_

#include <memory>  // for unique_ptr<>

#include <fable/conf.hpp>    // for Conf
#include <fable/json.hpp>    // for Json
#include <fable/schema.hpp>  // for Schema

#define CONFABLE_FRIENDS(xType)                                           \
  using ::fable::Confable::to_json;                                       \
  friend void to_json(::fable::Json& j, const xType& t) { t.to_json(j); } \
  friend void from_json(const ::fable::Json& j, xType& t) { t.from_conf(::fable::Conf{j}); }

#define CONFABLE_SCHEMA(xType) \
  CONFABLE_FRIENDS(xType)      \
  ::fable::Schema schema_impl() override

namespace fable {

class Confable {
 public:
  Confable() noexcept = default;

  Confable(const Confable&) noexcept : schema_(nullptr) {}
  Confable(Confable&&) noexcept : schema_(nullptr) {}

  Confable& operator=(const Confable& other) noexcept {
    if (this != &other) {
      schema_.reset();
    }
    return *this;
  }
  Confable& operator=(Confable&& other) noexcept {
    if (this != &other) {
      schema_.reset();
    }
    return *this;
  }

  virtual ~Confable() noexcept = default;

  /**
   * Reset the internal schema cache.
   *
   * This causes schema_impl to be called next time the schema is requested.
   */
  virtual void reset_schema() { schema_.reset(); }

  /**
   * Return the object schema for deserialization.
   *
   * This method uses `schema_impl` under the hood, which the object should
   * implement.
   */
  Schema& schema() {
    if (!schema_) {
      schema_ = std::make_unique<Schema>(schema_impl());
    }
    return *schema_.get();
  }

  /**
   * Return the object schema for description and validation.
   *
   * This method uses `schema_impl` under the hood, which the object should
   * implement.
   */
  const Schema& schema() const { return const_cast<Confable*>(this)->schema(); }

  /**
   * Validate a Conf without applying it.
   *
   * By default, this uses the validate() method on schema(). If you want to
   * guarantee anything extending beyond what's possible with schema, you can
   * do that here.
   *
   * This methodshould NOT call from_conf without also overriding from_conf
   * to prevent infinite recursion.
   */
  virtual void validate(const Conf& c) const { schema().validate(c); }

  /**
   * Deserialize a Confable from a Conf.
   *
   * Unless you have special needs, it is recommended to implement
   *
   *     Schema schema_impl()
   *
   * and use the default implementation of this.
   */
  virtual void from_conf(const Conf& c) {
    validate(c);
    schema().from_conf(c);
    reset_schema();
  }

  /**
   * Serialize a Confable to Json.
   *
   * Note: If you implement this type, make sure to either use CONFABLE_FRIENDS
   * or a using Confable::to_json statement in your derived type.
   */
  virtual void to_json(Json& j) const { schema().to_json(j); }

  /**
   * Serialize a Confable to Json.
   */
  Json to_json() const {
    Json j;
    to_json(j);
    return j;
  }

 protected:
  /**
   * Return a new instance of the schema for this object.
   *
   * This schema is then stored in schema_. This is called every time the
   * object is created or moved, since Schema contains references to fields
   * that (should be) in the object.
   */
  virtual Schema schema_impl() { return Schema(); }

 private:
  mutable std::unique_ptr<Schema> schema_{nullptr};
};

}  // namespace fable

namespace nlohmann {

template <>
struct adl_serializer<fable::Confable> {
  static void to_json(json& j, const fable::Confable& c) { c.to_json(j); }
  static void from_json(const json& j, fable::Confable& c) { c.from_conf(fable::Conf{j}); }
};

}  // namespace nlohmann

#endif  // FABLE_CONFABLE_HPP_
