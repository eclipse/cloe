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
 * \file fable/json.cpp
 * \see  fable/json.hpp
 */

#include <fable/json.hpp>

#include <string>  // for string

namespace fable {

std::string to_string(JsonType type) {
  switch (type) {
    case JsonType::null:
      return "null";
    case JsonType::object:
      return "object";
    case JsonType::array:
      return "array";
    case JsonType::boolean:
      return "boolean";
    case JsonType::number_float:
      return "number";
    case JsonType::number_integer:
      return "integer";
    case JsonType::number_unsigned:
      return "integer";
    case JsonType::string:
      return "string";
    default:
      return "unknown";
  }
}

}  // namespace fable
