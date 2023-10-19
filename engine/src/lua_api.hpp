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

/**
 * Return the cloe-engine table as it is exported into Lua.
 *
 * If you make any changes to these paths, make sure to reflect it:
 *
 *     engine/lua/cloe-engine/init.lua
 *
 */
[[nodiscard]] inline auto luat_cloe_engine(sol::state_view& lua) {
  return lua["package"]["loaded"]["cloe-engine"];
}

[[nodiscard]] inline auto luat_cloe_engine_fs(sol::state_view& lua) {
  return lua["package"]["loaded"]["cloe-engine.fs"];
}

[[nodiscard]] inline auto luat_cloe_engine_types(sol::state_view& lua) {
  return lua["package"]["loaded"]["cloe-engine.types"];
}

[[nodiscard]] inline auto luat_cloe_engine_initial_input(sol::state_view& lua) {
  return lua["package"]["loaded"]["cloe-engine"]["initial_input"];
}

[[nodiscard]] inline auto luat_cloe_engine_state(sol::state_view& lua) {
  return lua["package"]["loaded"]["cloe-engine"]["state"];
}

[[nodiscard]] inline auto luat_cloe_engine_plugins(sol::state_view& lua) {
  return lua["package"]["loaded"]["cloe-engine"]["plugins"];
}

}  // namespace cloe
