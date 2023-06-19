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
 * \file cloe/entity.cpp
 * \see  cloe/entity.hpp
 */

#include <cloe/entity.hpp>

#include <regex>  // for regex, regex_match

namespace cloe {

static std::regex VALID_NAME_REGEX{"^[a-zA-Z_][a-zA-Z0-9_]*(/[a-zA-Z_][a-zA-Z0-9_]*)*$"};
void Entity::set_name(std::string name) {
  if (!std::regex_match(name, VALID_NAME_REGEX)) {
    throw InvalidNameError(name);
  }
  name_ = std::move(name);
}

}  // namespace cloe
