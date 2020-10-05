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
#ifndef FABLE_JSON_HPP_
#define FABLE_JSON_HPP_

#include <string>  // for string

#include <nlohmann/json.hpp>  // for Json

namespace fable {

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
using JsonPointer = Json::json_pointer;

/**
 * The JsonType type maps to nlohmann::json::value_t.
 *
 * This makes it easier to talk/write about operations that deal on the
 * underlying type that is stored in a Json value.
 */
using JsonType = Json::value_t;

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

}  // namespace fable

#endif  // FABLE_JSON_HPP_
