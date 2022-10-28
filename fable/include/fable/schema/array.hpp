/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file fable/schema/array_fixed.hpp
 * \see  fable/schema/magic.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <limits>       // for numeric_limits<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string, stoul
#include <type_traits>  // for enable_if_t<>, is_convertible<>
#include <utility>      // for move
#include <vector>       // for vector<>

#include <fable/json.hpp>              // for Json
#include <fable/schema/interface.hpp>  // for Base<>, Box

namespace fable {
namespace schema {

template <typename T, size_t N, typename P>
class Array : public Base<Array<T, N, P>> {
 public:  // Types and Constructors
  using Type = std::array<T, N>;
  using PrototypeSchema = P;

  Array(Type* ptr, std::string&& desc);
  Array(Type* ptr, const PrototypeSchema& prototype)
      : Base<Array<T, N, P>>(), prototype_(prototype), ptr_(ptr) {}
  Array(Type* ptr, const PrototypeSchema& prototype, std::string&& desc)
      : Base<Array<T, N, P>>(std::move(desc)), prototype_(prototype), ptr_(ptr) {}

#if 0
  // This is defined in: fable/schema/magic.hpp
  Array(Type* ptr, std::string&& desc)
      : Array(ptr, make_prototype<T>(), std::move(desc)) {}
#endif

 public:  // Specials
  /**
   * Return whether deserialization must set all fields, in which case
   * only the array syntax is supported.
   *
   * By default, this is false.
   */
  bool require_all() const { return option_require_all_; }

  /**
   * Set whether deserialization should require_all the underlying array.
   */
  void set_require_all(bool value) {
    option_require_all_ = value;
    this->type_ = value ? JsonType::array : JsonType::null;
  }

  /**
   * Set whether deserialization should require_all the underlying array.
   */
  Array<T, N, P> require_all(bool value) && {
    this->set_require_all(value);
    return std::move(*this);
  }

 public:  // Overrides
  std::string type_string() const override { return "array of " + prototype_.type_string(); }

  Json json_schema() const override {
    Json j;
    if (option_require_all_) {
      j = this->json_schema_array();
    } else {
      j = Json::object({
          {"oneOf", Json::array({
                        this->json_schema_array(),
                        this->json_schema_object(),
                    })},
      });
    }

    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    if (option_require_all_) {
      this->validate_type(c);
      this->validate_array(c);
      return;
    }

    if (c->type() == JsonType::array) {
      this->validate_array(c);
    } else if (c->type() == JsonType::object) {
      this->validate_object(c);
    } else {
      this->throw_wrong_type(c);
    }
  }

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    assert(c->type() == JsonType::array || c->type() == JsonType::object);

    this->deserialize_into(c, *ptr_);
  }

  Json serialize(const Type& xs) const {
    Json j = Json::array();
    serialize_into(j, xs);
    return j;
  }

  /**
   * Serialize contents of `v` into `j`.
   */
  void serialize_into(Json& j, const Type& v) const {
    assert(j.type() == JsonType::array);
    for (const auto& x : v) {
      j.emplace_back(prototype_.serialize(x));
    }
  }

  /**
   * Deserialize the Conf into a new object.
   *
   * Because it's not pre-existing, we can't guarantee that it will be
   * initialized and therefore only support setting the full array.
   */
  Type deserialize(const Conf& c) const {
    Type array;
    this->deserialize_into(c, array);
    return array;
  }

  void deserialize_into(const Conf& c, Type& v) const {
    if (c->type() == JsonType::array) {
      this->deserialize_from_array(v, c);
    } else if (c->type() == JsonType::object) {
      this->deserialize_from_object(v, c);
    } else {
      this->throw_wrong_type(c);
    }
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  Json json_schema_array() const {
    return Json::object({
        {"type", "array"},
        {"items", prototype_.json_schema()},
        {"minItems", N},
        {"maxItems", N},
    });
  }

  Json json_schema_object() const {
    return Json::object({
        {"type", "object"},
        {"additionalProperties", false},
        {"patternProperties",
         {
             {"^[0-9]+$",
              {
                  {"type", prototype_.json_schema()},
              }},
         }},
    });
  }

  /**
   * Validate input that is an array that sets the entire array.
   *
   * It should have the format:
   *
   *    [ T0, T..., TN-1 ]
   *
   * That is, all elements in the array must be set, no less and
   * no more.
   */
  void validate_array(const Conf& c) const {
    assert(c->type() == JsonType::array);
    if (c->size() != N) {
      this->throw_error(c, "require exactly {} items in array, got {}", N, c->size());
    }
    for (const auto& x : c.to_array()) {
      prototype_.validate(x);
    }
  }

  /**
   * Validate input that is an object that indexes into the array.
   *
   * It should have the format:
   *
   *    {
   *      "^[0-9]+$": T
   *      ...
   *    }
   *
   * The index is converted to an integer with type size_t, which should be
   * less than the size N of the array. Otherwise an error is thrown.
   * More than one index can be set at once.
   */
  void validate_object(const Conf& c) const {
    assert(c->type() == JsonType::object);
    for (const auto& kv : c->items()) {
      const auto& key = kv.key();
      this->parse_index(c, key);
      prototype_.validate(c.at(key));
    }
  }

  /**
   * Parse the index from a string, throwing an error if it is not
   * a non-negative, base-10 integer of size less than N.
   *
   * You'd think this would be as easy as calling `std::stoul()`, but
   * unfortunately, that function comes with a bunch of defaults that we don't
   * want: strings like "234x" and "-0234" parse just fine, just not what
   * end-users necessarily expect.
   *
   * In the future, we may support indexing from the back of an array.
   * In that case, we can implement it in this method.
   */
  size_t parse_index(const Conf& c, const std::string& s) const {
    // We'd like to just be able to use std::stoul, but unfortunately
    // the standard library seems to think strings like "234x" are ok.
    if (s.empty()) {
      this->throw_error(c, "invalid index key in object, require integer, got ''");
    } else if (s.size() > 1 && s[0] == '0') {
      this->throw_error(c, "invalid index key in object, require base-10 value, got '{}'", s);
    }
    for (char ch : s) {
      if (ch < '0' || ch > '9') {
        this->throw_error(c, "invalid index key in object, require integer, got '{}'", s);
      }
    }
    size_t idx = std::stoul(s);
    if (idx >= N) {
      this->throw_error(c, "out-of-range index key in object, require < {}, got '{}'", N, s);
    }
    return idx;
  }

  [[noreturn]] void throw_wrong_type(const Conf& c) const {
    std::string got = to_string(c->type());
    this->throw_error(c, "property must have type array or object, got {}", got);
  }

  void deserialize_from_object(Type& array, const Conf& c) const {
    for (const auto& kv : c->items()) {
      const auto& key = kv.key();
      size_t idx = this->parse_index(c, key);
      prototype_.deserialize_into(c.at(key), array[idx]);
    }
  }

  void deserialize_from_array(Type& array, const Conf& c) const {
    auto src = c.to_array();
    size_t n = src.size();
    assert(n == N);
    for (size_t i = 0; i < n; i++) {
      array[i] = prototype_.deserialize(src[i]);
    }
  }

 private:
  bool option_require_all_{false};
  PrototypeSchema prototype_{};
  Type* ptr_{nullptr};
};

template <typename T, typename P, size_t N>
Array<T, N, P> make_schema(std::array<T, N>* ptr, const P& prototype, std::string&& desc) {
  return Array<T, N, P>(ptr, prototype, std::move(desc));
}

}  // namespace schema
}  // namespace fable
