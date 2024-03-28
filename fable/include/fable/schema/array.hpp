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
 * \file fable/schema/array.hpp
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

namespace fable::schema {

/**
 * The Array schema provides support for using the std::array<> schema.
 *
 * Because arrays are fixed size, there are two approaches to setting
 * data in them:
 *
 * 1. Setting the entire array, e.g. for arrays representing object vectors.
 * 2. Setting individual elements in array, e.g. for setting a boolean in a
 *    bit field.
 *
 * Example:
 *
 *     using Vec3d = std::array<double, 3>;
 *     Vec3d target;
 *     auto sma = fable::make_schema(&target, "position in 3d space");
 *
 * This will serialize to a JSON array:
 *
 *     [ 0.0, 0.0, 0.0 ]
 *
 * And it will deserialize from a JSON array:
 *
 *     [ 45.0, 22.0, 0.0 ]
 *
 * If you want to set the z-axis, you can use the object deserialization:
 *
 *     { "2": 5.0 }
 *
 * Note that, because we are using a JSON object, we need to specify the
 * index as string. The first index is "0" and the last is "N-1", where
 * N is the size of the array.
 *
 * Also note that setting a single element in the array does not modify any of
 * the other fields in the array, since the array exists outside of this
 * schema. In that regard, you are responsible for initializing your array.
 *
 * C Arrays
 * --------
 * If you have control over the definition of the variable, please use std::array<>.
 * Otherwise it is possible (though potentially undefined behavior) to
 * re-interpret the array as a std::array<>, since memory layout is identical.
 *
 * For example:
 *
 *     int c_array[] = {0, 0, 0};
 *     auto array_p = reinterpret_cast<std::array<int, 3>*>(c_array);
 *     static_assert(sizeof(c_array) == sizeof(array_p));
 *     auto schema = fable::make_schema(
 *       reintpret_cast<std::array<int, 3>*>(c_array),
 *       "schema description"
 *     );
 *
 * Any linter will probably tell you not to do this, so proceed at your own
 * risk.
 *
 * \see https://stackoverflow.com/questions/11205186/treat-c-cstyle-array-as-stdarray
 */
template <typename T, size_t N, typename P>
class Array : public Base<Array<T, N, P>> {
 public:  // Types and Constructors
  using Type = std::array<T, N>;
  using PrototypeSchema = P;

  Array(Type* ptr, std::string desc)
      : Array<T, N, P>(ptr, make_prototype<T>(), std::move(desc)) {}

  Array(Type* ptr, PrototypeSchema prototype)
      : Base<Array<T, N, P>>(), prototype_(std::move(prototype)), ptr_(ptr) {}

  Array(Type* ptr, PrototypeSchema prototype, std::string desc)
      : Base<Array<T, N, P>>(std::move(desc)), prototype_(std::move(prototype)), ptr_(ptr) {}

 public:  // Specials
  /**
   * Return whether deserialization must set all fields, in which case
   * only the full array syntax is supported.
   *
   * By default, this is false.
   */
  [[nodiscard]] bool require_all() const { return option_require_all_; }

  /**
   * Set whether deserialization requires setting the entire array.
   *
   * If set to true, then only the array syntax is supported:
   *
   *     [ 1, 3, 4]
   *
   * When set to false, then the object syntax is also supported:
   *
   *     { "1": 3 }
   *
   * The object syntax only sets the specified elements and leaves the others
   * untouched.
   *
   * Whether you choose one or the other depends on the semantics of your
   * data.
   */
  void set_require_all(bool value) {
    option_require_all_ = value;
    this->type_ = value ? JsonType::array : JsonType::null;
  }

  /**
   * Set whether deserialization requires setting the entire array.
   *
   * \see set_require_all(bool)
   */
  [[nodiscard]] Array<T, N, P> require_all(bool value) && {
    this->set_require_all(value);
    return std::move(*this);
  }

 public:  // Overrides
  [[nodiscard]] std::string type_string() const override { return "array of " + prototype_.type_string(); }

  [[nodiscard]] Json json_schema() const override {
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

  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    if (option_require_all_) {
      if (!this->validate_type(c, err)) {
        return false;
      }
      if (!this->validate_array(c, err)) {
        return false;
      }
      return true;
    }

    if (c->type() == JsonType::array) {
      return this->validate_array(c, err);
    } else if (c->type() == JsonType::object) {
      return this->validate_object(c, err);
    } else {
      return this->set_wrong_type(err, c);
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

  [[nodiscard]] Json serialize(const Type& xs) const {
    Json j = Json::array();
    serialize_into(j, xs);
    return j;
  }

  /**
   * Serialize contents of `v` into `j`.
   */
  void serialize_into(Json& j, const Type& v) const {
    assert(j.type() == JsonType::array);
    for (size_t i = 0; i < N; i++) {
      j.emplace_back(prototype_.serialize(v[i]));
    }
  }

  /**
   * Deserialize the Conf into a new object.
   *
   * Because it's not pre-existing, we can't guarantee that it will be
   * initialized and therefore only support setting the full array.
   */
  [[nodiscard]] Type deserialize(const Conf& c) const {
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
      throw this->wrong_type(c);
    }
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  [[nodiscard]] Json json_schema_array() const {
    return Json::object({
        {"type", "array"},
        {"items", prototype_.json_schema()},
        {"minItems", N},
        {"maxItems", N},
    });
  }

  [[nodiscard]] Json json_schema_object() const {
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
   * Validate input that is an array.
   *
   * It should have the format:
   *
   *    [ T0, T..., TN-1 ]
   *
   * That is, all elements in the array must be set, no less and
   * no more.
   */
  bool validate_array(const Conf& c, std::optional<SchemaError>& err) const {
    assert(c->type() == JsonType::array);
    if (c->size() != N) {
      return this->set_error(err, c, "require exactly {} items in array, got {}", N, c->size());
    }
    for (const auto& x : c.to_array()) {
      if (!prototype_.validate(x, err)) {
        return false;
      }
    }
    return true;
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
   * For example:
   *
   *    {
   *      "13": 31,
   *      "42": 24
   *    }
   *
   * The index is converted to an integer with type size_t, which should be
   * less than the size N of the array. Otherwise an error is thrown.
   * More than one index can be set at once. Unset indexes are left as is.
   */
  bool validate_object(const Conf& c, std::optional<SchemaError>& err) const {
    assert(c->type() == JsonType::object);
    for (const auto& kv : c->items()) {
      const auto& key = kv.key();
      this->parse_index(c, key);
      if (prototype_.validate(c.at(key), err)) {
        return false;
      }
    }
    return true;
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
      throw this->error(c, "invalid index key in object, require integer, got ''");
    } else if (s.size() > 1 && s[0] == '0') {
      throw this->error(c, "invalid index key in object, require base-10 value, got '{}'", s);
    }
    for (char ch : s) {
      if (ch < '0' || ch > '9') {
        throw this->error(c, "invalid index key in object, require integer, got '{}'", s);
      }
    }
    size_t idx = std::stoul(s);
    if (idx >= N) {
      throw this->error(c, "out-of-range index key in object, require < {}, got '{}'", N, s);
    }
    return idx;
  }

  [[nodiscard]] SchemaError wrong_type(const Conf& c) const {
    std::string got = to_string(c->type());
    return this->error(c, "property must have type array or object, got {}", got);
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
    if (src.size() != N) {
      throw this->error(c, "require exactly {} items in array, got {}", N, c->size());
    }
    for (size_t i = 0; i < N; i++) {
      array[i] = prototype_.deserialize(src[i]);
    }
  }

 private:
  bool option_require_all_{false};
  PrototypeSchema prototype_{};
  Type* ptr_{nullptr};
};

template <typename T, typename P, size_t N, typename S>
Array<T, N, P> make_schema(std::array<T, N>* ptr, P&& prototype, S&& desc) {
  return Array<T, N, P>(ptr, std::forward<P>(prototype), std::forward<S>(desc));
}

template <typename T, size_t N, typename S>
Array<T, N, decltype(make_prototype<T>())> make_schema(std::array<T, N>* ptr, S&& desc) {
  return Array<T, N, decltype(make_prototype<T>())>(ptr, std::forward<S>(desc));
}

}  // namespace fable::schema
