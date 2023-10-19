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

#include <fable/utility/sol.hpp>     // for Json(sol::object)
#include <fable/utility/string.hpp>  // for join_vector

#include "error_handler.hpp"  // for format_cloe_error
#include "lua_api.hpp"
#include "stack.hpp"
#include "utility/command.hpp"  // for CommandExecuter, CommandResult

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

void cloe_api_log(const std::string& level, const std::string& prefix, const std::string& msg) {
  auto lev = logger::into_level(level);
  auto log = cloe::logger::get(prefix.empty() ? prefix : "lua");
  log->log(lev, msg.c_str());
}

std::tuple<sol::object, sol::object> cloe_api_exec(sol::object obj, sol::this_state s) {
  // FIXME: This is not a very nice function...
  Command cmd;
  cmd.from_conf(fable::Conf{Json(obj)});

  engine::CommandExecuter exec(cloe::logger::get("lua"));
  auto result = exec.run_and_release(cmd);
  if (cmd.mode() != cloe::Command::Mode::Sync) {
    return {sol::lua_nil, sol::lua_nil};
  }
  sol::state_view lua(s);
  return {
      sol::object(lua, sol::in_place, fable::join_vector(result.output, "\n")),
      sol::object(lua, sol::in_place, *result.exit_code),
  };
}

template <typename T>
inline bool contains(const std::vector<T>& v, const T& x) {
  return std::find(v.begin(), v.end(), x) != v.end();
}

// Handle the exception.
//
// @param L     the lua state, which you can wrap in a state_view if necessary
// @param error the exception, if it exists
// @param desc  the what() of the exception or a description saying that we hit the general-case catch(...)
// @return Return value of sol::stack::push()
int lua_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception,
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

/**
 * Add package path to Lua search path.
 *
 * \see lua_setup_builtin.cpp
 */
void configure_package_path(sol::state_view& lua, const std::vector<std::string>& paths) {
  std::string package_path = lua["package"]["path"];
  for (const std::string& p : paths) {
    package_path += ";" + p + "/?.lua";
    package_path += ";" + p + "/?/init.lua";
  }
  lua["package"]["path"] = package_path;
}

/**
 * Add Lua package paths so that bundled Lua libaries can be found.
 */
void register_package_path(sol::state_view& lua, const LuaOptions& opt) {
  // Setup lua path:
  std::vector<std::string> lua_path{};
  if (!opt.no_system_lua) {
    // FIXME(windows): These paths are linux-specific.
    lua_path = {
        "/usr/local/lib/cloe/lua",
        "/usr/lib/cloe/lua",
    };
  }
  std::string lua_paths = opt.environment->get_or(CLOE_LUA_PATH, "");
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

  configure_package_path(lua, lua_path);
}

/**
 * Load "cloe-engine" library into Lua.
 *
 * This is then available via:
 *
 *     require("cloe-engine")
 *
 * Any changes you make here should be documented in the Lua meta files.
 *
 *     engine/lua/cloe-engine/init.lua
 */
void register_cloe_engine(sol::state_view& lua, Stack& stack) {
  sol::table tbl = lua.create_table();

  // Initial input will be processed at simulation start.
  tbl["initial_input"] = lua.create_table();
  tbl["initial_input"]["triggers"] = lua.create_table();
  tbl["initial_input"]["triggers_processed"] = 0;
  tbl["initial_input"]["signal_aliases"] = lua.create_table();
  tbl["initial_input"]["signal_requires"] = lua.create_table();

  // Plugin access will be made available by Coordinator.
  tbl["plugins"] = lua.create_table();

  // Simulation state will be extended in simulation.
  // clang-format off
  tbl["state"] = lua.create_table();
  tbl["state"]["report"] = lua.create_table();
  tbl["state"]["stack"] = std::ref(stack);
  tbl["state"]["scheduler"] = sol::lua_nil;
  tbl["state"]["current_script_file"] = sol::lua_nil;
  tbl["state"]["current_script_dir"] = sol::lua_nil;
  tbl["state"]["scripts_loaded"] = lua.create_table();
  tbl["state"]["features"] = lua.create_table_with(
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
    "cloe-server", CLOE_ENGINE_WITH_SERVER != 0,
    "cloe-lrdb", CLOE_ENGINE_WITH_LRDB != 0
  );
  // clang-format on

#if 0
  tbl.set_function("is_available", []() { return true; });
  tbl.set_function("get_script_file", [](sol::this_state lua) {
    return luat_cloe_engine_state(lua)["current_script_file"];
  });
  tbl.set_function("get_script_dir", [](sol::this_state lua) {
    return luat_cloe_engine_state(lua)["current_script_dir"];
  });
  tbl.set_function("get_report",
                   [](sol::this_state lua) { return luat_cloe_engine_state(lua)["report"]; });
  tbl.set_function("get_scheduler",
                   [](sol::this_state lua) { return luat_cloe_engine_state(lua)["scheduler"]; });
  tbl.set_function("get_features",
                   [](sol::this_state lua) { return luat_cloe_engine_state(lua)["features"]; });
  tbl.set_function("get_stack",
                   [](sol::this_state lua) { return luat_cloe_engine_state(lua)["stack"]; });
#endif
  tbl.set_function("log", cloe_api_log);
  tbl.set_function("exec", cloe_api_exec);

  luat_cloe_engine(lua) = tbl;
}

/**
 * Load "cloe-engine.types" library into Lua.
 *
 * This is then available via:
 *
 *     require("cloe-engine.types")
 *
 * Any changes you make here should be documented in the Lua meta files.
 *
 *     engine/lua/cloe-engine/types.lua
 */
void register_cloe_engine_types(sol::state_view& lua) {
  sol::table tbl = lua.create_table();
  register_usertype_duration(tbl);
  register_usertype_sync(tbl);
  register_usertype_stack(tbl);
  luat_cloe_engine_types(lua) = tbl;
}

/**
 * Load "cloe-engine.fs" library into Lua.
 *
 * This is then available via:
 *
 *     require("cloe-engine.fs")
 *
 * Any changes you make here should be documented in the Lua meta files:
 *
 *     engine/lua/cloe-engine/fs.lua
 */
void register_cloe_engine_fs(sol::state_view& lua) {
  sol::table tbl = lua.create_table();
  register_lib_fs(tbl);
  luat_cloe_engine_fs(lua) = tbl;
}

}  // anonymous namespace

sol::state new_lua(const LuaOptions& opt, Stack& stack) {
  // clang-format off
  sol::state lua;
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
  lua.set_exception_handler(&lua_exception_handler);
  // clang-format on

  register_package_path(lua, opt);
  register_cloe_engine(lua, stack);
  register_cloe_engine_types(lua);
  register_cloe_engine_fs(lua);
  return lua;
}

void merge_lua(sol::state_view& lua, const std::string& filepath) {
  logger::get("cloe")->debug("Load script {}", filepath);
  auto result = lua_safe_script_file(lua, std::filesystem::path(filepath));
  if (!result.valid()) {
    throw sol::error(result);
  }
}

}  // namespace cloe
