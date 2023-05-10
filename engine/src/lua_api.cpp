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
 * \file lua_api.cpp
 */

#include "lua_api.hpp"

#include <stdexcept>    // for exception

#include <sol/sol.hpp>  // for state_view

namespace cloe {

void load_lua_script(sol::state_view& lua, const std::filesystem::path& filepath) {
  auto file = std::filesystem::path(filepath);
  auto dir = file.parent_path().generic_string();
  if (dir == "") {
    dir = ".";
  }

  auto api = lua["cloe"]["api"];
  api["THIS_SCRIPT_FILE"] = file.generic_string();
  api["THIS_SCRIPT_DIR"] = dir;
  lua.safe_script_file(file.generic_string());
  api["THIS_SCRIPT_FILE"] = nullptr;
  api["THIS_SCRIPT_DIR"] = nullptr;
}

}  // namespace cloe
