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
 * \file lua_api.hpp
 * \see  lua_api.cpp
 * \see  lua_api_fs.cpp
 */

#pragma once

#include <sol/sol.hpp>  // for state_view
#include <filesystem>   // for path

namespace cloe {

/**
 * Safely load and run a user Lua script.
 */
sol::protected_function_result load_lua_script(sol::state_view& lua, const std::filesystem::path& filepath);

}  // namespace cloe
