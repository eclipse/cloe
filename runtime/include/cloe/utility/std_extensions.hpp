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
 * \file cloe/utility/std_extensions.hpp
 * \see  cloe/utility/std_extensions.cpp
 *
 * This file contains useful functions for dealing with standard datatypes.
 */

#pragma once
#ifndef CLOE_UTILITY_STD_EXTENSIONS_HPP_
#define CLOE_UTILITY_STD_EXTENSIONS_HPP_

#include <map>     // for map<>
#include <string>  // for string
#include <vector>  // for vector<>

namespace cloe {
namespace utility {

std::string join_vector(const std::vector<std::string>& v, const std::string& sep);

std::vector<std::string> split_string(std::string&& s, const std::string& sep);

template <typename T>
std::vector<std::string> map_keys(const std::map<std::string, T>& m) {
  std::vector<std::string> result;
  result.reserve(m.size());
  for (const auto& kv : m) {
    result.emplace_back(kv.first);
  }
  return result;
}

}  // namespace utility
}  // namespace cloe

#endif
