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

#include "stack.hpp"

#include <filesystem>          // for path
#include <sol/state_view.hpp>  // for state_view

// This variable is set from CMakeLists.txt, but in case it isn't,
// we will assume that the server is disabled.
#ifndef CLOE_ENGINE_WITH_SERVER
#define CLOE_ENGINE_WITH_SERVER 0
#endif

namespace cloe {

/**
 * Create `cloe.fs` table with filesystem functions.
 *
 * \see lua_api_fs.cpp
 */
sol::table make_cloe_fs_table(sol::state_view& lua);

namespace {

void setup_builtin(sol::state_view& lua) {
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

}  // anonymous namespace

void Stack::setup_lua() {
  setup_builtin(lua);

  // Add expected cloe-engine package paths.
  std::string package_path = lua["package"]["path"];
  for (const std::string& p : lua_path) {
    logger()->debug("Add Lua path: {}", p);
    package_path += ";" + p + "/?.lua";
  }
  lua["package"]["path"] = package_path;

  lua["cloe"] = lua.create_table();
  lua["cloe"]["fs"] = make_cloe_fs_table(lua);

  auto api = lua.create_table();

  auto experimental = lua.create_table();
  experimental.set_function("throw_exception", throw_exception);
  api["experimental"] = experimental;

  api["_FEATURES"] = make_cloe_api_features(lua);
  api.set_function("load_stackfile", [this](const std::string& filepath) {
    this->logger()->info("Include conf: {}", filepath);
    Conf config;
    try {
      config = this->conf_reader_func_(filepath);
    } catch (std::exception& e) {
      this->logger()->error("Error including conf {}: {}", filepath, e.what());
      throw;
    }
    from_conf(config);
  });

  lua["cloe"]["api"] = api;

  // Load cloe lua library extensions.
  // This should extend the cloe table we already defined here.
  lua.do_string("require('cloe')");
}

}  // namespace cloe