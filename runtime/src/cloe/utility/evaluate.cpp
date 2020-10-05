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
 * \file cloe/utility/evaluate.cpp
 * \see  cloe/utility/evaluate.hpp
 */

#include <cloe/utility/evaluate.hpp>

#include <algorithm>   // for less_equal, ...
#include <functional>  // for function<>, bind
#include <string>      // for string

#include <boost/lexical_cast.hpp>  // for lexical_cast<>

namespace cloe {
namespace utility {

std::function<bool(double)> compile_evaluation(const std::string& s) {
  std::string op;
  size_t pos = 0;
  for (auto ch : s) {
    if ((ch >= '0' && ch <= '9') || ch == '.') {
      break;
    }
    pos++;
    if (ch == ' ') {
      continue;
    }
    op.push_back(ch);
  }
  double num = boost::lexical_cast<double>(s.substr(pos));  // NOLINT
  return compile_evaluation(op, num);
}

std::function<bool(double)> compile_evaluation(const std::string& op, double num) {
  if (op == "==") {
    return std::bind(std::equal_to<double>(), std::placeholders::_1, num);
  } else if (op == "!=") {
    return std::bind(std::not_equal_to<double>(), std::placeholders::_1, num);
  } else if (op == "<") {
    return std::bind(std::less<double>(), std::placeholders::_1, num);
  } else if (op == "<=") {
    return std::bind(std::less_equal<double>(), std::placeholders::_1, num);
  } else if (op == ">") {
    return std::bind(std::greater<double>(), std::placeholders::_1, num);
  } else if (op == ">=") {
    return std::bind(std::greater_equal<double>(), std::placeholders::_1, num);
  } else {
    throw std::out_of_range("unknown operator '" + op + "'");
  }
}

}  // namespace utility
}  // namespace cloe
