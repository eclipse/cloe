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
 * \file fable/fable_fwd.hpp
 */

#if __cplusplus < 201703L
#error "the fable library requires a minimum standard of C++17"
#include <force_compiler_to_stop_here>
#endif

#include <string>

#include <nlohmann/detail/value_t.hpp>
#include <nlohmann/json_fwd.hpp>

namespace fable {

// from json.hpp
using Json = nlohmann::json;
using JsonPointer = nlohmann::json_pointer<std::string>;
using JsonType = nlohmann::detail::value_t;

// from conf.hpp
class Conf;

// from error.hpp
class Error;
class ConfError;
class SchemaError;

// from schema.hpp
class Schema;

// from confable.hpp
class Confable;

// from environment.hpp
class Environment;

}  // namespace fable
