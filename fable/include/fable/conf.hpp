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
 * \file fable/conf.hpp
 * \see  fable/conf.cpp
 *
 * This file contains Conf and ConfError, two classes which
 * aim to be a user and developer friendly approach to reading in JSON.
 *
 * One of the biggest challenges of reading a configuration is that of error
 * management. How do we ensure that we are getting the values that we need
 * when we need them, and in all other cases inform the user of their mistake.
 *
 * One approach to this is to verify each input via a schema. There is an RFC
 * currently being developed to add a JSON schema, but this poses two problems
 * for us:
 *
 *  1. A schema is overkill for smaller applications of reading configuration
 *     values and is thus developer unfriendly.
 *  2. It is not yet available.
 *
 * Another problem that needs to be solved in reading configurations is that
 * of relative paths. When a relative path is specified, the user expects
 * that the path is relative to the configuration being read. But once a JSON
 * has be deserialized, there is no intrinsic information telling the program
 * where that JSON file once resided, especially if we use a modular
 * configuration system. This means, we somehow need to add origin information
 * to configuration data. This is also necessary when we need to provide
 * user-friendly error messages.
 *
 * The Conf type attempts to address this problem by wrapping all
 * JSON data with a class that provides consistent and transparent error
 * message propagation and handling.
 */

#pragma once

#include <functional>  // for function<>
#include <string>      // for string
#include <vector>      // for vector<>
#include <filesystem>  // for path

#include <fmt/format.h>  // for fmt::format

#include <fable/fable_fwd.hpp>
#include <fable/json.hpp>  // for Json

namespace fable {

class ConfError;

class Conf {
 public:
  Conf() = default;
  explicit Conf(const Json& data) : data_(data) {}
  explicit Conf(const std::string& file);
  Conf(const Json& data, const std::string& file) : file_(file), data_(data) {}
  Conf(const Json& data, const std::string& file, const std::string& root)
      : file_(file), root_(root), data_(data) {}

  /**
   * Return whether this configuration was read from a file.
   *
   * - If not from a file, then the configuration must have come from one of:
   *   a) stdin
   *   b) command line
   *   c) network
   * - This method should not throw.
   */
  bool is_from_file() const { return !file_.empty(); }

  /**
   * Return the file associated with this configuration.
   */
  const std::string& file() const { return file_; }

  /**
   * Return whether this configuration is empty.
   */
  bool is_empty() const { return data_.is_null(); }

  /**
   * Return a reference to the JSON.
   *
   * This allows operations to be performed on the underlying type.
   */
  const Json& operator*() const { return data_; }
  Json& operator*() { return data_; }

  /**
   * Return a pointer to the JSON.
   *
   * This allows operations to be performed with the underlying type.
   */
  const Json* operator->() const { return &data_; }
  Json* operator->() { return &data_; }

  /**
   * Return the root of the current JSON as a JSON pointer.
   */
  std::string root() const { return (root_.empty() ? "/" : root_); }

  /**
   * Return whether the key is present in the configuration.
   *
   * - This method should not throw.
   */
  bool has(const std::string& key) const { return data_.count(key) != 0; }
  bool has(const JsonPointer& key) const;
  bool has_pointer(const std::string& key) const { return has(JsonPointer(key)); }

  /**
   * Return a new Conf basing off the JSON pointer.
   *
   * - Throws a ConfError if the key cannot be found.
   */
  Conf at(const std::string& key) const;
  Conf at(const JsonPointer& key) const;
  Conf at_pointer(const std::string& key) const { return at(JsonPointer(key)); }

  /**
   * Erase a key from the Conf if it exists and return the number of elements
   * removed.
   */
  size_t erase(const std::string& key);
  size_t erase(const JsonPointer& key);
  size_t erase_pointer(const std::string& key) { return erase(JsonPointer(key)); }

  /**
   * Return an array of Conf values.
   *
   * - Throws a ConfError if the object is not an array.
   */
  std::vector<Conf> to_array() const;

  /**
   * Return a value of type T.
   *
   * - Throws a ConfError if the key is of the wrong type.
   */
  template <typename T>
  T get() const {
    try {
      return data_.get<T>();
    } catch (nlohmann::detail::type_error&) {
      throw_wrong_type();
    }
  }

  /**
   * Return a value of type T at the position `key`.
   *
   * - Throws a ConfError if the key cannot be found or is wrong type.
   */
  template <typename T>
  T get(const std::string& key) const {
    try {
      return data_.at(key).get<T>();
    } catch (nlohmann::detail::out_of_range&) {
      throw_missing(key);
    } catch (nlohmann::detail::type_error&) {
      throw_wrong_type(key);
    }
  }

  template <typename T>
  T get(const JsonPointer& key) const {
    try {
      return data_.at(key).get<T>();
    } catch (nlohmann::detail::out_of_range&) {
      throw_missing(key.to_string());
    } catch (nlohmann::detail::type_error&) {
      throw_wrong_type(key.to_string());
    }
  }

  template <typename T>
  T get_pointer(const std::string& key) const {
    return get<T>(JsonPointer(key));
  }

  /**
   * Return a value of type T at the position `key`, returning `def` if the
   * key cannot be found.
   */
  template <typename T>
  T get_or(const std::string& key, T def) const {
    if (!data_.count(key)) {
      return def;
    }
    try {
      return data_.at(key).get<T>();
    } catch (nlohmann::detail::type_error&) {
      throw_wrong_type(key);
    }
  }

  template <typename T>
  T get_or(const JsonPointer& key, T def) const {
    try {
      return data_.at(key).get<T>();
    } catch (nlohmann::detail::out_of_range&) {
      return def;
    } catch (nlohmann::detail::type_error&) {
      throw_wrong_type(key.to_string());
    }
  }

  template <typename T>
  T get_pointer_or(const std::string& key, T def) const {
    return get_or(key, def);
  }

  /**
   * Perform the function on the value of the pointer `key`, if the key
   * can be found; otherwise the function is not executed.
   *
   * - Throws a ConfError if the value is of the wrong type.
   */
  template <typename T>
  void with(const std::string& key, std::function<void(const T&)> fn) const {
    if (data_.count(key)) {
      fn(get<T>(key));
    }
  }

  template <typename T>
  void with(const JsonPointer& key, std::function<void(const T&)> fn) const {
    try {
      fn(get<T>(key));
    } catch (nlohmann::detail::out_of_range&) {
      return;
    }
  }

  template <typename T>
  void with_pointer(const std::string& key, std::function<void(const T&)> fn) const {
    with(JsonPointer(key), fn);
  }

  /**
   * Write the value at the pointer `key` to val, if the key can be found.
   *
   * - Throws a ConfError if the value is of the wrong type.
   */
  template <typename T>
  void try_from(const std::string& key, T* val) const {
    if (data_.count(key)) {
      *val = get<T>(key);
    }
  }

  template <typename T>
  void try_from(const JsonPointer& key, T* val) const {
    try {
      *val = data_.at(key).get<T>();
    } catch (nlohmann::detail::out_of_range& e) {
      return;
    } catch (nlohmann::detail::type_error& e) {
      throw_wrong_type(key.to_string());
    }
  }

  template <typename T>
  void try_from_pointer(const std::string& key, T* val) const {
    try_from(JsonPointer(key), val);
  }

  /**
   * Assert that the pointer `key` resolves, throw a ConfError otherwise.
   */
  void assert_has(const std::string& key) const;
  void assert_has(const JsonPointer& key) const;
  void assert_has_pointer(const std::string& key) const { assert_has(JsonPointer(key)); }

  /**
   * Assert that the pointer `key` exists and resolves to the type `t`,
   * throw a ConfError otherwise.
   */
  void assert_has_type(const std::string& key, JsonType t) const;
  void assert_has_type(const JsonPointer& key, JsonType t) const;
  void assert_has_pointer_type(const std::string& key, JsonType t) const {
    assert_has_type(JsonPointer(key), t);
  }

  /**
   * Assert that the pointer `key` is not available, throw a ConfError
   * otherwise.
   */
  void assert_has_not(const std::string& key, const std::string& msg = "") const;
  void assert_has_not(const JsonPointer& key, const std::string& msg = "") const;
  void assert_has_pointer_not(const std::string& key, const std::string& msg = "") const {
    assert_has_not(JsonPointer(key), msg);
  }

  /**
   * Resolve a path to an absolute path by taking the configuration file
   * into account.
   *
   * - If the path is absolute, return as is.
   * - If the path is relative, but file is stdin, return relative to
   *   execution.
   * - If the path is relative and file is not stdin, return relative to
   *   the file.
   */
  std::filesystem::path resolve_file(const std::filesystem::path& filename) const;
  std::string resolve_file(const std::string& filename) const;

  template <typename... Args>
  [[noreturn]] void throw_error(std::string_view format, Args&&... args) const {
    throw_error(fmt::format(format, std::forward<Args>(args)...));
  }
  [[noreturn]] void throw_error(const std::string& msg) const;
  [[noreturn]] void throw_unexpected(const std::string& key, const std::string& msg = "") const;
  [[noreturn]] void throw_missing(const std::string& key) const;
  [[noreturn]] void throw_wrong_type(const std::string& key = "") const;
  [[noreturn]] void throw_wrong_type(const std::string& key, JsonType type) const;

  friend void to_json(Json& j, const Conf& c) { j = *c; }
  friend void from_json(const Json& j, Conf& c) { *c = j; }

 private:
  std::string file_;
  std::string root_;
  Json data_;
};

}  // namespace fable
