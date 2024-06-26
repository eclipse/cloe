/*
 * Copyright 2024 Robert Bosch GmbH
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

#include <gtest/gtest.h>

#include <cstdio>  // for tmpnam
#include <filesystem>
#include <fstream>
#include <string_view>

#include <sol/state.hpp>

#include "lua_api.hpp"    // for lua_safe_script_file
#include "lua_setup.hpp"  // for setup_lua
#include "stack.hpp"      // for Stack
using namespace cloe;     // NOLINT(build/namespaces)

#ifndef CLOE_LUA_PATH
#error "require CLOE_LUA_PATH to be defined in order to find lua directory"
#endif

class cloe_lua_setup : public testing::Test {
  std::vector<std::filesystem::path> defer_deletion_;

 protected:
  sol::state lua_state;
  sol::state_view lua;
  LuaOptions opt;
  Stack stack;

  cloe_lua_setup() : lua(lua_state.lua_state()) {
    opt.environment = std::make_unique<fable::Environment>();
#ifdef CLOE_LUA_PATH
    opt.lua_paths.emplace_back(CLOE_LUA_PATH);
#endif
  }

  std::filesystem::path WriteTempLuaFile(std::string_view content) {
    // NOTE: Regarding the danger of std::tmpname.
    // It is required to have a real file in the filesystem that can be
    // then laoded by lua.safe_script_file. Because tests are run in parallel,
    // this filename needs to be reasonably random. Since this program is not
    // running at high privileges, the potential attack vector should not result
    // in an escalation of privileges.
    auto temp_file = std::filesystem::path(std::string(std::tmpnam(nullptr)) + ".lua");  // NOLINT
    std::ofstream ofs(temp_file);
    if (!ofs.is_open()) {
      throw std::ios_base::failure("Failed to create temporary file");
    }
    ofs << content;
    ofs.close();

    defer_deletion_.push_back(temp_file);
    return temp_file;
  }

  void TearDown() override {
    for (const auto& f : defer_deletion_) {
      std::filesystem::remove(f);
    }
  }
};

TEST_F(cloe_lua_setup, cloe_engine_is_available) {
  setup_lua(lua, opt, stack);
  lua.script(R"(
    local api = require("cloe-engine")
    assert(api.is_available())
    assert(not api.is_simulation_running())
  )");
}

TEST_F(cloe_lua_setup, describe_cloe) {
  setup_lua(lua, opt, stack);
  ASSERT_EQ(lua.script("local cloe = require('cloe'); return cloe.inspect(cloe.LogLevel.CRITICAL)").get<std::string>(),
            std::string("\"critical\""));
}

TEST_F(cloe_lua_setup, describe_cloe_without_require) {
  opt.auto_require_cloe = true;
  setup_lua(lua, opt, stack);
  ASSERT_EQ(lua.script("return cloe.inspect(cloe.LogLevel.CRITICAL)").get<std::string>(),
            std::string("\"critical\""));
}

TEST_F(cloe_lua_setup, read_engine_state) {
  setup_lua(lua, opt, stack);
  ASSERT_TRUE(lua.script("return require('cloe-engine').is_available()").get<bool>());
  ASSERT_FALSE(luat_cloe_engine_state(lua)["is_running"].get<bool>());
}

TEST_F(cloe_lua_setup, write_engine_state) {
  setup_lua(lua, opt, stack);
  luat_cloe_engine_state(lua)["extra"] = "hello world!";
  ASSERT_EQ(lua.script("return require('cloe-engine').state.extra").get<std::string>(),
            std::string("hello world!"));
}

TEST_F(cloe_lua_setup, write_engine_state_table) {
  setup_lua(lua, opt, stack);
  sol::table state = luat_cloe_engine_state(lua);
  sol::table scripts_loaded = state["scripts_loaded"];
  ASSERT_TRUE(scripts_loaded.valid());
  scripts_loaded[scripts_loaded.size() + 1] = "hello_world.lua";
  ASSERT_EQ(lua.script("return require('cloe-engine').state.scripts_loaded[1]").get<std::string>(),
            std::string("hello_world.lua"));
}

TEST_F(cloe_lua_setup, lua_safe_script_file) {
  auto file = WriteTempLuaFile(R"(
    local cloe = require("cloe")
    local api = require("cloe-engine")
    return api.get_script_file()
  )");
  setup_lua(lua, opt, stack);
  ASSERT_EQ(lua_safe_script_file(lua, file).get<std::string>(), file.generic_string());
}
