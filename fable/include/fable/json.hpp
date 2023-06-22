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
 * \file fable/json.hpp
 *
 * This file provides the basic `fable::Json` type used to represent JSON data.
 * For functions and other `to_json` functions, see files in the `json/` directory.
 */

#pragma once

#include <string>  // for string

#include <nlohmann/json.hpp>  // for Json

namespace fable {

/**
 * When parsing JSON from fable, should comments be allowed or lead to an
 * exception? Value is controlled by `PARSE_JSON_WITH_COMMENTS` define.
 * Default value is true.
 *
 * This does not automatically apply to use of `Json::parse`. Instead you
 * should use the parse_json helper function defined in this file.
 */
extern bool NLOHMANN_JSON_ALLOW_COMMENTS;

/**
 * When parsing JSON from fable, should exceptions be used?
 * Default value is set by `PARSE_JSON_WITH_COMMENTS` define.
 *
 * This is here for completeness. The fable library will not work
 * correctly if exceptions are disabled, so this should always be set to true.
 */
extern bool NLOHMANN_JSON_USE_EXCEPTIONS;

/**
 * The Json type maps to nlohmann::json.
 *
 * Bringing it into the fable namespace makes it much easier to use.
 * Making it start with capital J prevents any collision with builtins.
 * Because it is a using statement, standard nlohmann::json constructs can
 * be used, such as to_json and from_json.
 */
using Json = nlohmann::json;

/**
 * The JsonPointer type maps to nlohmann::json_pointer.
 *
 * This makes it easier to talk/write about operations that deal with
 * JSON pointers.
 */
using JsonPointer = nlohmann::json_pointer<std::string>;

/**
 * The JsonType type maps to nlohmann::json::value_t.
 *
 * This makes it easier to talk/write about operations that deal on the
 * underlying type that is stored in a Json value.
 */
using JsonType = nlohmann::detail::value_t;

/**
 * Return a string representation of a JSON type.
 *
 * The returned string is one of the following values:
 *
 *     null
 *     object
 *     array
 *     boolean
 *     float
 *     integer
 *     unsigned
 *     string
 *     unknown
 */
std::string to_string(JsonType);

/**
 * Return the result of Json::parse, with the options to use exceptions and
 * allow comments as set in this library, from NLOHMANN_JSON_USE_EXCEPTIONS and
 * NLOHMANN_JSON_ALLOW_COMMENTS.
 *
 * See the documentation on each of these global variables for more details.
 */
template <typename InputType>
Json parse_json(InputType&& input) {
  return Json::parse(std::forward<InputType>(input), nullptr, NLOHMANN_JSON_USE_EXCEPTIONS,
                     NLOHMANN_JSON_ALLOW_COMMENTS);
}

}  // namespace fable
