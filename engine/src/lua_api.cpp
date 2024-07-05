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

#include "lua_api.hpp"

#include <filesystem>  // for path

#include <cloe/core/logger.hpp>  // for logger::get
#include <sol/state_view.hpp>    // for state_view

namespace cloe {

sol::protected_function_result lua_safe_script_file(sol::state_view lua,
                                                    const std::filesystem::path& filepath) {
  auto file = std::filesystem::path(filepath);
  auto dir = file.parent_path().generic_string();
  if (dir.empty()) {
    dir = ".";
  }

  sol::object old_file = luat_cloe_engine_state(lua)["current_script_file"];
  sol::object old_dir = luat_cloe_engine_state(lua)["current_script_dir"];
  sol::table scripts_loaded = luat_cloe_engine_state(lua)["scripts_loaded"];
  scripts_loaded[scripts_loaded.size() + 1] = file.generic_string();
  luat_cloe_engine_state(lua)["scripts_loaded"] = scripts_loaded;
  luat_cloe_engine_state(lua)["current_script_file"] = file.generic_string();
  luat_cloe_engine_state(lua)["current_script_dir"] = dir;
  logger::get("cloe")->info("Loading {}", file.generic_string());
  auto result = lua.safe_script_file(file.generic_string(), sol::script_pass_on_error);
  luat_cloe_engine_state(lua)["current_script_file"] = old_file;
  luat_cloe_engine_state(lua)["current_script_dir"] = old_dir;
  return result;
}

}  // namespace cloe
