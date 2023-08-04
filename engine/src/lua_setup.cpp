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
 * \file stack_lua.cpp
 */

#include "lua_setup.hpp"

#include <filesystem>  // for path

#include <sol/state_view.hpp>  // for state_view

#include <cloe/utility/std_extensions.hpp>  // for split_string

#include "error_handler.hpp"  // for format_cloe_error
#include "lua_api.hpp"
#include "stack.hpp"

// This variable is set from CMakeLists.txt, but in case it isn't,
// we will assume that the server is disabled.
#ifndef CLOE_ENGINE_WITH_SERVER
#define CLOE_ENGINE_WITH_SERVER 0
#endif

#ifndef CLOE_LUA_PATH
#define CLOE_LUA_PATH "CLOE_LUA_PATH"
#endif

namespace cloe {

namespace {

sol::table make_cloe_api_features(sol::state_view& lua) {
  // clang-format off
  return lua.create_table_with(
    // Version compatibility:
    "cloe-0.18.0", true,
    "cloe-0.18", true,
    "cloe-0.19.0", true,
    "cloe-0.19", true,
    "cloe-0.20.0", true,
    "cloe-0.20", true,
    "cloe-0.21.0", true, // nightly
    "cloe-0.21", true,   // nightly

    // Stackfile versions support:
    "cloe-stackfile", true,
    "cloe-stackfile-4", true,
    "cloe-stackfile-4.0", true,
    "cloe-stackfile-4.1", true,

    // Server enabled:
    "cloe-server", CLOE_ENGINE_WITH_SERVER
  );
  // clang-format on
}

void throw_exception(const std::string& msg) { throw cloe::Error(msg); }

void cloe_api_log(const std::string& level, const std::string& prefix, const std::string& msg) {
  auto lev = logger::into_level(level);
  auto log = cloe::logger::get(prefix.empty() ? prefix : "lua");
  log->log(lev, msg.c_str());
}

template <typename T>
inline bool contains(const std::vector<T>& v, const T& x) {
  return std::find(v.begin(), v.end(), x) != v.end();
}

}  // anonymous namespace

// Handle the exception.
//
// @param L     the lua state, which you can wrap in a state_view if necessary
// @param error the exception, if it exists
// @param desc  the what() of the exception or a description saying that we hit the general-case catch(...)
// @return Return value of sol::stack::push()
int cloe_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception,
                           sol::string_view desc) {
  if (maybe_exception) {
    const std::exception& err = *maybe_exception;
    std::cerr << "Error: " << format_error(err) << std::endl;
  } else {
    std::cerr << "Error: ";
    std::cerr.write(desc.data(), static_cast<std::streamsize>(desc.size()));
    std::cerr << std::endl;
  }

  // you must push 1 element onto the stack to be
  // transported through as the error object in Lua
  // note that Lua -- and 99.5% of all Lua users and libraries
  // -- expects a string so we push a single string (in our
  // case, the description of the error)
  return sol::stack::push(L, desc);
}

void setup_lua(sol::state_view& lua, Stack& stack) {
  lua.set_exception_handler(&cloe_exception_handler);

  // Create cloe table
  {
    sol::table cloe_tbl = lua.create_table();
    register_usertype_duration(cloe_tbl);
    register_usertype_sync(cloe_tbl);
    register_usertype_stack(cloe_tbl);

    sol::table fs_tbl = lua.create_table();
    register_lib_fs(fs_tbl);
    cloe_tbl["fs"] = fs_tbl;

    sol::table state_tbl = lua.create_table();
    state_tbl["stack"] = std::ref(stack);
    state_tbl["features"] = make_cloe_api_features(lua);
    cloe_tbl["state"] = state_tbl;

    sol::table api_tbl = lua.create_table();
    api_tbl.set_function("log", cloe_api_log);

    sol::table experimental_tbl = lua.create_table();
    experimental_tbl.set_function("throw_exception", throw_exception);
    api_tbl["experimental"] = experimental_tbl;

    cloe_tbl["api"] = api_tbl;
    lua["cloe"] = cloe_tbl;
  }

  // Load cloe lua library extensions.
  // This should extend the cloe table we already defined here.
  auto result = lua.safe_script("require('cloe')");
  if (!result.valid()) {
    throw static_cast<sol::error>(result);
  }
}

sol::state new_lua(const LuaOptions& opt, Stack& s) {
  sol::state lua;

  // Setup lua path:
  std::vector<std::string> lua_path{};
  if (!opt.no_system_lua) {
    // FIXME(windows): These paths are linux-specific.
    lua_path = {
        "/usr/local/lib/cloe/lua",
        "/usr/lib/cloe/lua",
    };
  }
  std::string lua_paths = opt.environment.get()->get_or(CLOE_LUA_PATH, "");
  for (auto&& p : utility::split_string(std::move(lua_paths), ":")) {
    if (contains(lua_path, p)) {
      continue;
    }
    lua_path.emplace_back(std::move(p));
  }
  for (const auto& p : opt.lua_paths) {
    if (contains(lua_path, p)) {
      continue;
    }
    lua_path.emplace_back(p);
  }

  register_builtins(lua);
  configure_package_path(lua, lua_path);
  setup_lua(lua, s);
  return lua;
}

void merge_lua(sol::state_view& lua, const std::string& filepath) {
  logger::get("cloe")->debug("Load script {}", filepath);
  auto result = lua_safe_script_file(lua, std::filesystem::path(filepath));
  if (!result.valid()) {
    sol::error err = result;
    throw err;
  }
}

}  // namespace cloe
