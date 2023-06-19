/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file fable/utility/string.cpp
 * \see  fable/utility/string.hpp
 */

#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector<>

namespace fable {

std::string join_vector(const std::vector<std::string>& v, std::string_view sep) {
  std::string result;
  size_t n = v.size();
  for (size_t i = 1; i <= n; i++) {
    result += v[i - 1];
    if (i < n) {
      result += sep;
    }
  }
  return result;
}

std::vector<std::string> split_string(std::string_view s) {
  std::vector<std::string> results;
  for (char ch : s) {
    results.emplace_back(std::string(1, ch));
  }
  return results;
}

std::vector<std::string> split_string(std::string_view s, std::string_view sep) {
  if (sep.empty()) {
    return split_string(s);
  }

  std::vector<std::string> results;
  while (s.size() > 0) {
    size_t pos = s.find(sep);
    results.emplace_back(s.substr(0, pos));
    if (pos == std::string_view::npos) {
      break;
    }
    s = s.substr(pos + sep.size());
  }
  return results;
}

}  // namespace fable
