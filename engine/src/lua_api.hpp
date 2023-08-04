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
 * This file contains functions for dealing with Lua *after* it has been
 * set up.
 *
 * \file lua_api.hpp
 * \see  lua_api.cpp
 */

#pragma once

#include <filesystem>  // for std::filesystem::path

#include <sol/protected_function_result.hpp>  // for protected_function_result
#include <sol/state_view.hpp>                 // for state_view

namespace cloe {

/**
 * Safely load and run a user Lua script.
 */
[[nodiscard]] sol::protected_function_result lua_safe_script_file(
    sol::state_view& lua, const std::filesystem::path& filepath);

}  // namespace cloe
