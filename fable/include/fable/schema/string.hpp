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
 * \file fable/schema/string.hpp
 * \see  fable/schema/string.cpp
 * \see  fable/schema/string_test.cpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <limits>   // for numeric_limits<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {

// Forward declarations:
class Environment;  // from <fable/environment.hpp>

namespace schema {

/**
 * \macro FABLE_REGEX_C_PATTERN specifies the regex for the
 * String::c_identifier() method.
 *
 * Overriding it will not have any effect.
 */
#define FABLE_REGEX_C_IDENTIFIER "^[a-zA-Z_][a-zA-Z0-9_]*$"

/**
 * String de-/serializes a string.
 *
 * A string can have the following validation properties set:
 *
 * - minimum length (in bytes)
 * - maximum length (in bytes)
 * - regular expression pattern
 * - with or without ENV variable interpolation
 * - with or withot a special environment for interpolation
 *
 * The String schema type allows these properties to be specified
 * and will validate them during deserialization.
 */
class String : public Base<String> {
 public:  // Types and Constructors
  using Type = std::string;

  String(Type* ptr, std::string&& desc) : Base(JsonType::string, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  /**
   * Ensure input is not empty if present.
   *
   * This is effectively an alias for `min_length(1)`.
   *
   * \return *this, for chaining
   */
  String not_empty() &&;

  /**
   * Return the current minimum length (0 if unset).
   *
   * \return minimum string length in bytes
   */
  size_t min_length() const;

  /**
   * Set the minimum string length in bytes.
   *
   * \param value min bytes
   * \return *this, for chaining
   */
  String min_length(size_t value) &&;

  /**
   * Set the minimum string length in bytes.
   *
   * \param value min bytes
   */
  void set_min_length(size_t value);

  /**
   * Return the current maximum length (max size_t value if unset).
   *
   * \return maximum string length in bytes
   */
  size_t max_length() const;

  /**
   * Set the maximum string length in bytes.
   *
   * \param value max bytes
   * \return *this, for chaining
   */
  String max_length(size_t value) &&;

  /**
   * Set the maximum string length in bytes.
   *
   * \param value maximum string length in bytes
   */
  void set_max_length(size_t value);

  /**
   * Return the regular expression pattern the string should match.
   *
   * \return regex pattern
   */
  const std::string& pattern() const;

  /**
   * Set the string regular expression pattern.
   *
   * \param value regex pattern
   * \return *this, for chaining
   */
  String pattern(const std::string& value) &&;

  /**
   * Set the string regular expression pattern.
   *
   * \param value regex pattern
   */
  void set_pattern(const std::string& value);

  /**
   * Ensure that the input matches an identifier as roughly specified
   * by C.
   *
   * This is shorthand for setting the pattern to FABLE_REGEX_C_IDENTIFIER.
   */
  String c_identifier() &&;

  /**
   * Return whether shell-style variable interpolation is enabled (default false).
   *
   * \return true if enabled
   */
  bool interpolate() const;

  /**
   * \copydoc String::set_interpolate(bool)
   *
   * \return *this, for chaining
   */
  String interpolate(bool value) &&;

  /**
   * Set whether variable interpolation is enabled.
   *
   * The following strings are then interpolated:
   *
   * This uses the `Environment` set by the `set_environment()` method.
   * If no environment is set, a default empty environment is used with OS environment
   * fallback.
   *
   * The string `"${SHELL}"` for example will evaluate to the current shell as defined
   * in the OS environment, unless explicitely set in an `Environment` that is passed
   * to `set_environment()`.
   *
   * Alternatives can be provided, such as with `"${NOT_EXIST-alternate string}"`.
   *
   * \see Environment
   * \see String::set_environment(Environment)
   *
   * \param value true to enable
   */
  void set_interpolate(bool value);

  /**
   * Return the Environment used for variable interpolation.
   *
   * \return environment
   */
  Environment* environment() const;

  /**
   * Set the Environment used for variable interpolation.
   *
   * \see String::set_interpolate(bool)
   *
   * \param environment
   * \return *this, for chaining
   */
  String environment(Environment* env) &&;

  /**
   * Set the Environment used for variable interpolation.
   *
   * \see String::set_interpolate(bool)
   *
   * \param environment
   */
  void set_environment(Environment* env);

  /**
   * Return the set of valid values for the string.
   *
   * \return valid values
   */
  const std::vector<std::string>& enum_of() const;

  /**
   * Set the valid values for the string.
   *
   * \note If a pattern is set, the string will need to validate
   * against the pattern in addition to this list.
   *
   * \param init list of valid values
   * \return *this, for chaining
   */
  String enum_of(std::vector<std::string>&& init);

  /**
   * Set the valid values for the string.
   *
   * \note If a pattern is set, the string will need to validate
   * against the pattern in addition to this list.
   *
   * \param init list of valid values
   */
  void set_enum_of(std::vector<std::string>&& init);

 public:  // Overrides
  Json json_schema() const override;
  void validate(const Conf& c) const override;
  using Interface::to_json;
  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override;
  Json serialize(const Type& x) const;
  Type deserialize(const Conf& c) const;
  void reset_ptr() override;

 private:
  bool interpolate_{false};
  size_t min_length_{0};
  size_t max_length_{std::numeric_limits<size_t>::max()};
  std::string pattern_{};
  std::vector<std::string> enum_{};
  Environment* env_{nullptr};
  Type* ptr_{nullptr};
};

inline String make_schema_impl(std::string* ptr, std::string&& desc) {
  return String(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable
