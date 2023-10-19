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
 * This file contains function definitions required to set up the Lua API.
 *
 * \file lua_setup.hpp
 */

#pragma once

#include <iostream>  // for ostream, cerr
#include <memory>    // for shared_ptr<>
#include <optional>  // for optional<>
#include <string>    // for string
#include <vector>    // for vector<>

#include <sol/state.hpp>
#include <sol/state_view.hpp>
#include <sol/table.hpp>

#include <fable/environment.hpp>  // for Environment

namespace cloe {

class Stack;

struct LuaOptions {
  std::shared_ptr<fable::Environment> environment;

  std::vector<std::string> lua_paths;
  bool no_system_lua = false;
};

/**
 * Create a new lua state.
 *
 * Currently this requires a fully configured Stack file.
 *
 * \see cloe::new_stack()
 * \see stack_factory.hpp
 * \see lua_setup.cpp
 */
sol::state new_lua(const LuaOptions& opt, Stack& s);

#if CLOE_ENGINE_WITH_LRDB
/**
 * Start Lua debugger server on port.
 *
 * \param lua
 * \param listen_port
 */
void start_lua_debugger(sol::state& lua, int listen_port);
#endif

/**
 * Merge the provided Lua file into the existing `Stack`, respecting `StackOptions`.
 *
 * \see lua_setup.cpp
 */
void merge_lua(sol::state_view& lua, const std::string& filepath);

/**
 * Define the filesystem library functions in the given table.
 *
 * The following functions are made available:
 *
 *  - basename
 *  - dirname
 *  - normalize
 *  - realpath
 *  - join
 *  - is_absolute
 *  - is_relative
 *  - is_dir
 *  - is_file
 *  - is_other
 *  - exists
 *
 * \see lua_setup_fs.cpp
 */
void register_lib_fs(sol::table& lua);

/**
 * Define `cloe::Duration` usertype in Lua.
 *
 * \see cloe/core/duration.hpp from cloe-runtime
 * \see lua_setup_duration.cpp
 */
void register_usertype_duration(sol::table& lua);

/**
 * Define `cloe::Sync` usertype in Lua.
 *
 * \see cloe/sync.hpp from cloe-runtime
 * \see lua_setup_sync.cpp
 */
void register_usertype_sync(sol::table& lua);

/**
 * Define `cloe::Stack` usertype in Lua.
 *
 * \see clua_setup_stack.cpp
 */
void register_usertype_stack(sol::table& lua);

}  // namespace cloe
