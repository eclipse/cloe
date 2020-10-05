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
 * \file cloe/utility/evaluate.hpp
 * \see  cloe/utility/evaluate.cpp
 */

#pragma once
#ifndef CLOE_UTILITY_EVALUATE_HPP_
#define CLOE_UTILITY_EVALUATE_HPP_

#include <functional>  // for function<>
#include <string>      // for string

namespace cloe {
namespace utility {

/**
 * Compile an evaluation string into a function that evaluates a single double.
 *
 * For example:
 *
 * - "<50" returns a function that returns true when input is less than 50.
 * - ">60" returns a function that returns true when input is greater than 60.
 *
 * An `out_of_range` error is thrown if the operator is not one of the
 * following: ==, !=, <, <=, >, >=
 */
std::function<bool(double)> compile_evaluation(const std::string& s);
std::function<bool(double)> compile_evaluation(const std::string& op, double val);

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_EVALUATE_HPP_
