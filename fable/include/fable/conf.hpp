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

#include <filesystem>  // for path
#include <functional>  // for function<>
#include <string>      // for string
#include <utility>     // for move
#include <vector>      // for vector<>

#include <fmt/format.h>  // for fmt::format

#include <fable/fable_fwd.hpp>  // for ConfError
#include <fable/json.hpp>       // for Json

namespace fable {

/**
 * Conf wraps a JSON with context that allows for more user-friendly errors.
 *
 * In general, a Conf wraps JSON content that comes from the filesystem and
 * is used for configuration purposes. Even if a class requires only a subset
 * of the JSON, Conf maintains the connection to the original file, which
 * allows errors to be returned that refer to the file and location.
 *
 * Conf is best used together with the Confable and Schema types.
 *
 * This class provides many methods that take JSON pointers, either as strings
 * (these methods have `pointer` in the name to differentiate them from the
 * plain versions) or as JsonPointer.
 *
 * For more information, see: https://datatracker.ietf.org/doc/html/rfc6901
 */
class Conf {
 public:
  Conf() = default;
  explicit Conf(Json data) : data_(std::move(data)) {}
  explicit Conf(std::string file);
  Conf(Json data, std::string file) : file_(std::move(file)), data_(std::move(data)) {}
  Conf(Json data, std::string file, std::string root)
      : file_(std::move(file)), root_(std::move(root)), data_(std::move(data)) {}

  /**
   * Return whether this configuration was read from a file.
   *
   * - If not from a file, then the configuration must have come from one of:
   *   a) stdin
   *   b) command line
   *   c) network
   * - This method should not throw.
   */
  [[nodiscard]] bool is_from_file() const { return !file_.empty(); }

  /**
   * Return the file associated with this configuration.
   */
  [[nodiscard]] const std::string& file() const { return file_; }

  /**
   * Return whether this configuration is empty.
   */
  [[nodiscard]] bool is_empty() const { return data_.is_null(); }

  /**
   * Return a reference to the JSON.
   *
   * This allows operations to be performed on the underlying Json type.
   */
  const Json& operator*() const { return data_; }

  /**
   * Return a reference to the JSON.
   *
   * This allows operations to be performed on the underlying Json type.
   */
  Json& operator*() { return data_; }

  /**
   * Return a pointer to the JSON.
   *
   * This allows operations to be performed with the underlying Json type.
   */
  const Json* operator->() const { return &data_; }

  /**
   * Return a pointer to the JSON.
   *
   * This allows operations to be performed with the underlying Json type.
   */
  Json* operator->() { return &data_; }

  /**
   * Return the root of the current JSON formatted as a JSON pointer.
   *
   * When a Conf is initially created, it has a root of '/'.
   * When `at()` is used to index into the Conf, a new Conf is returned with
   * the root of that new object set to `/` plus the field.
   * So if the root is '/foo', and you use `at("bar")`, then the new root
   * is `/foo/bar`.
   *
   * This is primarily used for creating better error messages when something
   * goes wrong.
   *
   * \returns string of Conf root formatted as a JSON pointer
   */
  [[nodiscard]] std::string root() const { return (root_.empty() ? "/" : root_); }

  /**
   * Return whether the field is present.
   *
   * Notes:
   * - This method should not throw.
   * - A field with value 'null' counts as present.
   *
   * \param key target field to check for existence
   * \returns true if present, false otherwise
   */
  [[nodiscard]] bool has(const std::string& key) const { return data_.count(key) != 0; }

  /**
   * Return whether the field referred to by JSON pointer is present.
   *
   * Notes:
   * - This method should not throw.
   * - A field with value 'null' counts as present.
   *
   * \param ptr JSON pointer to target field to check for existence
   * \returns true if present, false otherwise
   */
  [[nodiscard]] bool has(const JsonPointer& ptr) const;

  /**
   * Return whether the field referred to by JSON pointer is present.
   *
   * Notes:
   * - A field with value 'null' counts as present.
   *
   * \param ptr JSON pointer to target field to check for existence
   * \returns true if present, false otherwise
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  [[nodiscard]] bool has_pointer(const std::string& ptr) const { return has(JsonPointer(ptr)); }

  /**
   * Return a new Conf basing off the JSON at the target field.
   *
   * The root of the new Conf will contain the current root + key.
   *
   * \param key target field to access
   * \returns Conf of JSON if it exists
   * \throws ConfError if the key cannot be found
   */
  [[nodiscard]] Conf at(const std::string& key) const;

  /**
   * Return a new Conf basing off the JSON referred to by the JSON pointer.
   *
   * The root of the new Conf will contain the current root + ptr.
   *
   * \param ptr JSON pointer to target field, e.g. "/foo/bar"
   * \returns Conf of JSON if it exists
   * \throws ConfError if the ptr cannot be resolved
   */
  [[nodiscard]] Conf at(const JsonPointer& ptr) const;

  /**
   * Return a new Conf basing off the JSON referred to by the JSON pointer.
   *
   * The root of the new Conf will contain the current root + ptr.
   *
   * \param ptr JSON pointer to target field, e.g. "/foo/bar"
   * \returns Conf of JSON if it exists
   * \throws ConfError if the ptr cannot be resolved
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  [[nodiscard]] Conf at_pointer(const std::string& ptr) const { return at(JsonPointer(ptr)); }

  /**
   * Erase a field from the Conf if it exists and return 1 or 0.
   *
   * Notes:
   * - The object itself cannot be erased with a key of "".
   * - This method should not throw.
   *
   * \param key target field to erase
   * \returns number of elements removed
   */
  size_t erase(const std::string& key);

  /**
   * Erase a field from the Conf if it exists and return number of elements removed.
   *
   * When removing the last element in an object, erase will by default then
   * remove the whole object. This can lead to a cascade, resulting in multiple
   * elements being removed. You can inhibit this behavior by passing true to
   * the `preserve_empty` parameter.
   *
   * Notes:
   * - The root object ("/") cannot be erased.
   * - This method should not throw.
   *
   * \param ptr JSON pointer to target field to remove
   * \param preserve_empty whether to preserve empty objects
   * \returns number of elements removed
   */
  size_t erase(const JsonPointer& ptr, bool preserve_empty = false);

  /**
   * Erase a field from the Conf if it exists and return number of elements removed.
   *
   * When removing the last element in an object, erase will by default then
   * remove the whole object. This can lead to a cascade, resulting in multiple
   * elements being removed. You can inhibit this behavior by passing true to
   * the `preserve_empty` parameter.
   *
   * Notes:
   * - The root object ("/") cannot be erased.
   *
   * \param ptr JSON pointer to target field to remove
   * \param preserve_empty whether to preserve empty objects
   * \returns number of elements removed
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  size_t erase_pointer(const std::string& ptr, bool preserve_empty = false) {
    return erase(JsonPointer(ptr), preserve_empty);
  }

  /**
   * Return an array of Conf values.
   *
   * This is useful when you want to iterate through individual Conf values
   * (for better error reporting) instead of Json values.
   *
   * Example:
   *
   *     for (auto&& c : conf.to_array()) {
   *       // do something with c
   *     }
   *
   * \throws ConfError if the object is not an array
   */
  [[nodiscard]] std::vector<Conf> to_array() const;

  /**
   * Return a value of type T.
   *
   * \throws ConfError if the key is of the wrong type
   */
  template <typename T>
  [[nodiscard]] T get() const {
    try {
      return data_.get<T>();
    } catch (Json::type_error&) {
      throw_wrong_type();
    }
  }

  /**
   * Return a value of type T at the position `key`.
   *
   * \param key target field to get
   * \throws ConfError if the key cannot be found or is wrong type
   */
  template <typename T>
  [[nodiscard]] T get(const std::string& key) const {
    try {
      return data_.at(key).get<T>();
    } catch (Json::out_of_range&) {
      throw_missing(key);
    } catch (Json::type_error&) {
      throw_wrong_type(key);
    }
  }

  /**
   * Return a value of type T at the pointer `ptr`.
   *
   * \param ptr JSON pointer to target field to get
   * \throws ConfError if the ptr cannot be found or is wrong type
   */
  template <typename T>
  [[nodiscard]] T get(const JsonPointer& ptr) const {
    try {
      return data_.at(ptr).get<T>();
    } catch (Json::out_of_range&) {
      throw_missing(ptr);
    } catch (Json::type_error&) {
      throw_wrong_type(ptr);
    }
  }

  /**
   * Return a value of type T at the pointer `ptr`.
   *
   * \param ptr JSON pointer to target field to get
   * \throws ConfError if the ptr cannot be found or is wrong type
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  template <typename T>
  [[nodiscard]] T get_pointer(const std::string& ptr) const {
    return get<T>(JsonPointer(ptr));
  }

  /**
   * Return a value of type T for the field `key`, returning `def` if the
   * field cannot be found.
   *
   * \param key target field to get
   * \param def default value if target field does not exist
   * \throws ConfError if target Json is of wrong type
   */
  template <typename T>
  [[nodiscard]] T get_or(const std::string& key, T def) const {
    if (!data_.count(key)) {
      return def;
    }
    try {
      return data_.at(key).get<T>();
    } catch (Json::type_error&) {
      throw_wrong_type(key);
    }
  }

  /**
   * Return a value of type T at the pointer `ptr`, returning `def` if the
   * field cannot be found.
   *
   * \param ptr JSON pointer to target field to get
   * \param def default value if target field does not exist
   * \throws ConfError if target Json is of wrong type
   */
  template <typename T>
  [[nodiscard]] T get_or(const JsonPointer& ptr, T def) const {
    try {
      return data_.at(ptr).get<T>();
    } catch (Json::out_of_range&) {
      return def;
    } catch (Json::type_error&) {
      throw_wrong_type(ptr);
    }
  }

  /**
   * Return a value of type T at the pointer `ptr`, returning `def` if the
   * field cannot be found.
   *
   * \param ptr JSON pointer to target field to get
   * \param def default value if target field does not exist
   * \throws ConfError if target Json is of wrong type
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  template <typename T>
  [[nodiscard]] T get_pointer_or(const std::string& ptr, T def) const {
    return get_or(JsonPointer(ptr), def);
  }

  /**
   * Perform the function on the value of the field `key`, if the field
   * can be found; otherwise the function is not executed.
   *
   * \throws ConfError if the value is of the wrong type
   */
  template <typename T>
  void with(const std::string& key, std::function<void(const T&)> fn) const {
    if (data_.count(key)) {
      fn(get<T>(key));
    }
  }

  /**
   * Perform the function on the value of the pointer `ptr`, if the field
   * can be found; otherwise the function is not executed.
   *
   * \throws ConfError if the value is of the wrong type
   */
  template <typename T>
  void with(const JsonPointer& ptr, std::function<void(const T&)> fn) const {
    try {
      fn(get<T>(ptr));
    } catch (Json::out_of_range&) {
      return;
    }
  }

  /**
   * Perform the function on the value of the pointer `ptr`, if the field
   * can be found; otherwise the function is not executed.
   *
   * \throws ConfError if the value is of the wrong type
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  template <typename T>
  void with_pointer(const std::string& ptr, std::function<void(const T&)> fn) const {
    with(JsonPointer(ptr), fn);
  }

  /**
   * Write the value at the field `key` to val, if the key can be found.
   *
   * If the field cannot be found, the value is not modified.
   *
   * \param[in] key target field to get data from
   * \param[out] val target variable to write value to
   * \throws ConfError if the value is of the wrong type
   */
  template <typename T>
  void try_from(const std::string& key, T& val) const {
    if (data_.count(key)) {
      val = get<T>(key);
    }
  }

  /**
   * Write the value at the pointer `key` to val, if the key can be found.
   *
   * If the field cannot be found, the value is not modified.
   *
   * \param[in] ptr JSON pointer to target field to get data from
   * \param[out] val target variable to write value to
   * \throws ConfError if the value is of the wrong type
   */
  template <typename T>
  void try_from(const JsonPointer& ptr, T& val) const {
    try {
      val = data_.at(ptr).get<T>();
    } catch (Json::out_of_range& e) {
      return;
    } catch (Json::type_error& e) {
      throw_wrong_type(ptr);
    }
  }

  /**
   * Write the value at the pointer `key` to val, if the key can be found.
   *
   * If the field cannot be found, the value is not modified.
   *
   * \param[in] ptr JSON pointer to target field to get data from
   * \param[out] val target variable to write value to
   * \throws a ConfError if the value is of the wrong type
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  template <typename T>
  void try_from_pointer(const std::string& ptr, T& val) const {
    try_from(JsonPointer(ptr), val);
  }

  /**
   * Assert that the field `key` resolves, throw a ConfError otherwise.
   *
   * \param key target field to check
   * \throws ConfError if field does not exist
   */
  void assert_has(const std::string& key) const;

  /**
   * Assert that the pointer `ptr` resolves, throw a ConfError otherwise.
   *
   * \param ptr JSON pointer to check
   * \throws ConfError if field does not exist
   */
  void assert_has(const JsonPointer& ptr) const;

  /**
   * Assert that the pointer `ptr` resolves, throw a ConfError otherwise.
   *
   * \param ptr JSON pointer to check
   * \throws ConfError if field does not exist
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  void assert_has_pointer(const std::string& key) const { assert_has(JsonPointer(key)); }

  /**
   * Assert that the field `key` exists and resolves to the type `t`,
   * throw a ConfError otherwise.
   *
   * \param key target field to access
   * \param t type target value should have
   * \throws ConfError if field non-existent or is wrong type
   */
  void assert_has_type(const std::string& key, JsonType t) const;

  /**
   * Assert that the pointer `ptr` exists and resolves to the type `t`,
   * throw a ConfError otherwise.
   *
   * \param ptr JSON pointer to target field, e.g. "/foo/bar"
   * \param t type target value should have
   * \throws ConfError if field non-existent or is wrong type
   */
  void assert_has_type(const JsonPointer& ptr, JsonType t) const;

  /**
   * Assert that the pointer `ptr` exists and resolves to the type `t`,
   * throw a ConfError otherwise.
   *
   * \param ptr JSON pointer to target field, e.g. "/foo/bar"
   * \param t type target value should have
   * \throws ConfError if field non-existent or is wrong type
   * \throws Json::parse_error if ptr is not a valid JSON pointer
   */
  void assert_has_pointer_type(const std::string& ptr, JsonType t) const {
    assert_has_type(JsonPointer(ptr), t);
  }

  /**
   * Assert that the field `key` is not available, throw a ConfError
   * otherwise.
   *
   * \param key target field to check for non-existence
   * \param msg optional custom error message (use to explain what AND why)
   */
  void assert_has_not(const std::string& key, const std::string& msg = "") const;

  /**
   * Assert that the field given by the JSON pointer is not available,
   * throw a ConfError otherwise.
   *
   * \param ptr JSON pointer to target field to check for non-existence
   * \param msg optional custom error message (use to explain what AND why)
   * \throws ConfError if field exists
   */
  void assert_has_not(const JsonPointer& ptr, const std::string& msg = "") const;

  /**
   * Assert that the field given by the JSON pointer is not available,
   * throw a ConfError otherwise.
   *
   * \param ptr JSON pointer to target field to check for non-existence
   * \param msg optional custom error message (use to explain what AND why)
   * \throws ConfError if field exists
   */
  void assert_has_pointer_not(const std::string& ptr, const std::string& msg = "") const {
    assert_has_not(JsonPointer(ptr), msg);
  }

  /**
   * Resolve a path to an absolute path by taking the configuration file
   * into account.
   *
   * - If the path is absolute, return as is.
   * - If the path is relative, but file is stdin, return relative to
   *   current working directory.
   * - If the path is relative and file is not stdin, return relative to
   *   the file.
   */
  [[nodiscard]] std::filesystem::path resolve_file(const std::filesystem::path& filename) const;

  /**
   * Resolve a path to an absolute path by taking the configuration file
   * into account.
   *
   * - If the path is absolute, return as is.
   * - If the path is relative, but file is stdin, return relative to
   *   current working directory.
   * - If the path is relative and file is not stdin, return relative to
   *   the file.
   */
  [[nodiscard]] std::string resolve_file(const std::string& filename) const;

  /**
   * Throw a ConfError with the Conf data as the context.
   *
   * The format string and arguments passed are formated with fmt.
   *
   * \param format format string according to fmt
   * \param args arguments for format string
   * \throws ConfError
   */
  template <typename... Args>
  [[noreturn]] void throw_error(std::string_view format, Args&&... args) const {
    throw_error(fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
  }

  /**
   * Throw a ConfError with the Conf data as the context and the message provided.
   * \throws ConfError
   */
  [[noreturn]] void throw_error(const std::string& msg) const;

  /**
   * Throw a ConfError explaining the named field is unexpected,
   * with the Conf data as the context.
   *
   * \param msg optional message to replace default message
   * \throws ConfError
   */
  [[noreturn]] void throw_unexpected(const std::string& key, const std::string& msg = "") const;

  /**
   * Throw a ConfError explaining the named field is unexpected,
   * with the Conf data as the context.
   *
   * \param msg optional message to replace default message
   * \throws ConfError
   */
  [[noreturn]] void throw_unexpected(const JsonPointer& ptr, const std::string& msg = "") const;

  /**
   * Throw a ConfError explaining the named field is missing,
   * with the Conf data as the context.
   *
   * \throws ConfError
   */
  [[noreturn]] void throw_missing(const std::string& key) const;

  /**
   * Throw a ConfError explaining the named field is missing,
   * with the Conf data as the context.
   *
   * \throws ConfError
   */
  [[noreturn]] void throw_missing(const JsonPointer& ptr) const;

  /**
   * Throw a ConfError explaining that the named field has the wrong type,
   * with the Conf data as the context.
   *
   * \throws ConfError
   */
  [[noreturn]] void throw_wrong_type(const std::string& key = "") const;

  /**
   * Throw a ConfError explaining that the named field has the wrong type,
   * with the Conf data as the context.
   *
   * \throws ConfError
   */
  [[noreturn]] void throw_wrong_type(const JsonPointer& ptr) const;

  /**
   * Throw a ConfError explaining that the named field has the wrong type,
   * with the Conf data as the context.
   *
   * \param expected JSON type that is expected
   * \throws ConfError
   */
  [[noreturn]] void throw_wrong_type(const std::string& key, JsonType expected) const;

  /**
   * Throw a ConfError explaining that the named field has the wrong type,
   * with the Conf data as the context.
   *
   * \param expected JSON type that is expected
   * \throws ConfError
   */
  [[noreturn]] void throw_wrong_type(const JsonPointer& ptr, JsonType expected) const;

  /**
   * Conversion function for automatic "serialization" of Conf in Json.
   *
   * This fulfills the interface provided by nlohmann::json.
   */
  friend void to_json(Json& j, const Conf& c) { j = *c; }

  /**
   * Conversion function for automatic "deserialization" of Json in Conf.
   *
   * This fulfills the interface provided by nlohmann::json.
   */
  friend void from_json(const Json& j, Conf& c) { *c = j; }

 private:
  std::string file_;
  std::string root_;
  Json data_;
};

}  // namespace fable
