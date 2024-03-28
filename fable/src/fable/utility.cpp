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
 * \file fable/utility.cpp
 * \see  fable/utility.hpp
 */

#include <fable/utility.hpp>

#include <algorithm>  // for replace
#include <fstream>    // for ifstream
#include <iostream>   // for cin, ostream
#include <iterator>   // for istreambuf_iterator
#include <stdexcept>  // for runtime_error

#include <fable/environment.hpp>  // for Environment
#include <fable/error.hpp>        // for ConfError, SchemaError

namespace fable {

Json read_json_from_file(const char* filepath) {
  std::ifstream ifs(filepath);
  if (ifs.fail()) {
    throw std::runtime_error(std::string("could not open file: ") + filepath);
  }

  return parse_json(ifs);
}

Json read_json_from_file(const std::string& filepath) {
  return read_json_from_file(filepath.c_str());
}

Json read_json_from_stdin() {
  Json j;
  std::cin >> j;
  return j;
}

Json read_json(const std::string& filepath_or_stdin) {
  if (filepath_or_stdin == "-") {
    return read_json_from_stdin();
  } else {
    return read_json_from_file(filepath_or_stdin);
  }
}

Json read_json_with_interpolation(const std::string& filepath_or_stdin, const Environment* env) {
  if (filepath_or_stdin == "-") {
    std::istreambuf_iterator<char> begin(std::cin), end;
    std::string s(begin, end);
    s = interpolate_vars(s, env);
    return parse_json(s);
  } else {
    std::ifstream ifs(filepath_or_stdin);
    if (ifs.fail()) {
      throw std::runtime_error(std::string("could not open file: ") + filepath_or_stdin);
    }
    std::istreambuf_iterator<char> begin(ifs), end;
    std::string s(begin, end);
    s = interpolate_vars(s, env);
    return parse_json(s);
  }
}

Conf read_conf_from_file(const char* filepath) { return Conf{std::string(filepath)}; }

Conf read_conf_from_file(const std::string& filepath) {
  return read_conf_from_file(filepath.c_str());
}
Conf read_conf_from_stdin() { return Conf{read_json_from_stdin()}; }

Conf read_conf(const std::string& filepath_or_stdin) {
  if (filepath_or_stdin == "-") {
    return Conf{read_json_from_stdin()};
  } else {
    return read_conf_from_file(filepath_or_stdin);
  }
}

Conf read_conf_with_interpolation(const std::string& filepath_or_stdin, const Environment* env) {
  if (filepath_or_stdin == "-") {
    return Conf{read_json_with_interpolation("-", env)};
  } else {
    return Conf{read_json_with_interpolation(filepath_or_stdin, env), filepath_or_stdin};
  }
}

std::string indent_string(std::string s, const std::string& indent) {
  std::string search = "\n";
  std::string replace = "\n" + indent;
  size_t pos = 0;
  while ((pos = s.find(search, pos)) != std::string::npos) {
      s.replace(pos, search.length(), replace);
      pos += replace.length();
  }
  return indent + s;
}

#define PRETTY_PRINT_INDENT "        "

void pretty_print(const ConfError& e, std::ostream& os) {
  os << e.message() << std::endl
     << "    In JSON segment:\n"
     << indent_string(e.data().dump(2), PRETTY_PRINT_INDENT) << std::endl;
}

void pretty_print(const SchemaError& e, std::ostream& os) {
  os << e.message() << std::endl
     << "    In JSON segment:\n"
     << indent_string(e.data().dump(2), PRETTY_PRINT_INDENT) << std::endl
     << "    With following JSON schema:\n"
     << indent_string(e.schema().dump(2), PRETTY_PRINT_INDENT) << std::endl;
  if (!e.context().empty()) {
    os << "    With following error context:\n"
       << indent_string(e.context().dump(2), PRETTY_PRINT_INDENT) << std::endl;
  }
}

}  // namespace fable
