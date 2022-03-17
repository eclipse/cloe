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
 * \file cloe/utility/inja.hpp
 */

#pragma once
#ifndef CLOE_UTILITY_INJA_HPP_
#define CLOE_UTILITY_INJA_HPP_

#include <utility>  // for move

#include <inja/inja.hpp>  // for Environment

namespace cloe {
namespace utility {

/**
 * Return an inja environment with which to render files.
 *
 * We change the default expression, comment, and statement delimiters to use
 * square braces instead of curly braces, since we have configuration files
 * that are sent through the real Jinja first.
 */
inline inja::Environment inja_env() {
  inja::Environment env;
  env.set_expression("[[", "]]");
  env.set_comment("[#", "#]");
  env.set_statement("[%", "%]");
  return std::move(env);
}

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_INJA_HPP_
