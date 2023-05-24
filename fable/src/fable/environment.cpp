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
 * \file      fable/environment.cpp
 * \see       fable/environment.hpp
 * \see       fable/environment_test.cpp
 */

#include <fable/environment.hpp>

#include <cstdlib>    // for getenv
#include <stdexcept>  // for out_of_range
#include <string>     // for string

namespace fable {

namespace {

std::string slice(const std::string& s, size_t start, size_t end) {
  return s.substr(start, end - start);
}

std::string slice(const std::string& s, size_t start) { return s.substr(start); }

}  // anonymous namespace

std::optional<std::string> Environment::get(const std::string& key, bool prefer_external) const {
  std::optional<std::string> value;

  auto try_external = [&]() -> bool {
    char* s = std::getenv(key.c_str());
    if (s != nullptr) {
      value = std::string(s);
      return true;
    }
    return false;
  };

  auto try_internal = [&]() -> bool {
    if (defines_.count(key)) {
      value = defines_.at(key);
      return true;
    }
    return false;
  };

  if (prefer_external) {
    try_external() || try_internal();
  } else {
    try_internal() || try_external();
  }

  return value;
}

std::string Environment::require(const std::string& key, bool prefer_external) const {
  auto value = get(key, prefer_external);
  if (!value) {
    throw std::out_of_range("environment variable does not exist: " + key);
  }
  return *value;
}

std::string Environment::evaluate(const std::string& s, bool prefer_external,
                                  bool allow_undefined) const {
  size_t pos = s.find("-");
  std::string raw_key = (pos == std::string::npos ? s : slice(s, 0, pos));
  std::string key = interpolate(raw_key, prefer_external, allow_undefined);
  auto value = get(key, prefer_external);
  if (value) {
    return *value;
  } else if (pos != std::string::npos) {
    // The alternative will ONLY be processed if the initial key cannot be
    // found. This is not only good for avoiding work, it provides a kind of
    // shortcutting that allows alternatives to not be valid when the primary
    // key is available.
    return interpolate(slice(s, pos + 1), prefer_external, allow_undefined);
  } else if (allow_undefined) {
    return "";
  } else {
    if (raw_key == key) {
      throw std::out_of_range("environment variable does not exist: " + key);
    } else {
      throw std::out_of_range("nested environment variable does not exist: " + raw_key + " -> " +
                              key);
    }
  }
}

std::string Environment::interpolate(const std::string& s, bool prefer_external,
                                     bool allow_undefined) const {
  const size_t n = s.size();

  std::string output;
  size_t start = 0;
  size_t pos = 0;
  size_t depth = 0;
  while (pos != n) {
    switch (s[pos]) {
      case '$':
        if (pos + 1 == n) {
          throw std::invalid_argument("unterminated $ in string, expect one of $ or {");
        }
        switch (s[pos + 1]) {
          case '$':
            // Squash $$ into $ and write it to output.
            output += slice(s, start, pos);
            start = pos + 1;
            break;
          case '{':
            // Write text up until now to output and increase depth.
            if (depth == 0) {
              output += slice(s, start, pos);
              start = pos;
            }
            depth++;
            break;
          default:
            throw std::invalid_argument("unescaped $ in string, use $$ for a literal $");
        }
        pos++;
        break;
      case '}':
        if (depth > 0) {
          depth--;
          if (depth == 0) {
            auto token = slice(s, start + 2, pos);
            output += evaluate(token, prefer_external, allow_undefined);
            start = pos + 1;
          }
        }
        break;
      default:
        break;
    }
    pos++;
  }

  if (depth != 0) {
    throw std::invalid_argument("unterminated variable in string");
  }
  if (start != pos) {
    output += slice(s, start, pos);
  }
  return output;
}

}  // namespace fable
