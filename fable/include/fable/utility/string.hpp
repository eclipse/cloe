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
 * \file fable/utility/string.hpp
 * \see  fable/utility/string.cpp
 */

#pragma once

#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector<>

namespace fable {

inline bool starts_with(std::string_view s, std::string_view prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(std::string_view s, std::string_view suffix) {
  return s.size() >= suffix.size() &&
         s.compare(s.size() - suffix.size(), std::string::npos, suffix) == 0;
}

std::string join_vector(const std::vector<std::string>& v, std::string_view sep);

/**
 * Split string into vector by a separator.
 *
 * - The separator is stripped from the output.
 * - Multiple separators result in empty strings.
 * - An empty separator results in a vector of individual characters.
 *
 * \param s input string view
 * \param sep separator to split by
 */
std::vector<std::string> split_string(std::string_view s, std::string_view sep);

}  // namespace fable
