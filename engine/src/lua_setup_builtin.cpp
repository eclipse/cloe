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

#include "lua_setup.hpp"

#include <sol/state_view.hpp>

namespace cloe {

void configure_package_path(sol::state_view& lua, const std::vector<std::string>& paths) {
  std::string package_path = lua["package"]["path"];
  for (const std::string& p : paths) {
    package_path += ";" + p + "/?.lua";
    package_path += ";" + p + "/?/init.lua";
  }
  lua["package"]["path"] = package_path;
}

void register_builtins(sol::state_view& lua) {
  // clang-format off
  lua.open_libraries(
    sol::lib::base,
    sol::lib::coroutine,
    sol::lib::debug,
    sol::lib::io,
    sol::lib::math,
    sol::lib::os,
    sol::lib::package,
    sol::lib::string,
    sol::lib::table
  );
  // clang-format on
}

}  // namespace lua
