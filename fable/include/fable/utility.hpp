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

#include <string>  // for string
#include <ostream> // for ostream

#include <fable/fable_fwd.hpp>

namespace fable {

/**
 * Read a file and parse the contents to JSON.
 *
 * If the file does not exist, a std::runtime_error is thrown.
 * If the JSON is malformed, a std::invalid_argument is thrown.
 */
Json read_json_from_file(const char* filepath);

Json read_json_from_stdin();

Json read_json_from_file(const std::string& filepath);

Json read_json(const std::string& filepath_or_stdin);

Json read_json_with_interpolation(const std::string& filepath_or_stdin,
                                  const Environment* env = nullptr);

Conf read_conf_from_file(const char* filepath);

Conf read_conf_from_stdin();

Conf read_conf_from_file(const std::string& filepath);

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
