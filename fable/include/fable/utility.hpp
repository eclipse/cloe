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
 * \file fable/utility.hpp
 * \see  fable/utility.cpp
 */

#pragma once
#ifndef FABLE_UTILITY_HPP_
#define FABLE_UTILITY_HPP_

#include <string>  // for string

#include <fable/conf.hpp>  // for Conf
#include <fable/json.hpp>  // for Json

namespace fable {

// Forward declarations:
class ConfError;    // from <fable/error.hpp>
class SchemaError;  // from <fable/error.hpp>
class Environment;  // from <fable/environment.hpp>

/**
 * Read a file and parse the contents to JSON.
 *
 * If the file does not exist, a std::runtime_error is thrown.
 * If the JSON is malformed, a std::invalid_argument is thrown.
 */
Json read_json_from_file(const char* filepath);

Json read_json_from_stdin();

inline Json read_json_from_file(const std::string& filepath) {
  return read_json_from_file(filepath.c_str());
}

Json read_json(const std::string& filepath_or_stdin);

Json read_json_with_interpolation(const std::string& filepath_or_stdin,
                                  const Environment* env = nullptr);

Conf read_conf_from_file(const char* filepath);

Conf read_conf_from_stdin();

inline Conf read_conf_from_file(const std::string& filepath) {
  return read_conf_from_file(filepath.c_str());
}

Conf read_conf(const std::string& filepath_or_stdin);

Conf read_conf_with_interpolation(const std::string& filepath_or_stdin,
                                  const Environment* env = nullptr);

/**
 * Indent a string in its entirety by the value of indent.
 */
std::string indent_string(std::string s, const std::string& indent = "    ");

void pretty_print(const ConfError& e, std::ostream& os);

void pretty_print(const SchemaError& e, std::ostream& os);

}  // namespace fable

#endif  // FABLE_UTILITY_HPP_
