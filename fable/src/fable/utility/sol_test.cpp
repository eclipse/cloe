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

#include <chrono>

#include <gtest/gtest.h>
#include <sol/sol.hpp>

#include <fable/utility/sol.hpp>
#include <fable/utility/gtest.hpp>
#include <fable/json.hpp>

using namespace fable;

TEST(fable_utility_sol, to_json) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  auto assert_xeq = [&](const char* lua_script, const char* expect) {
    lua.script(lua_script);
    assert_eq(sol::object(lua["x"]), expect);
  };

  assert_xeq("x = true", "true");
  assert_xeq("x = 1", "1");
  assert_xeq("x = 2.5", "2.5");
  assert_xeq("x = { false, true }", "[ false, true ]");
  assert_xeq("x = 'hello world'", "\"hello world\"");
  assert_xeq("x = { name = 'ok', value = 42 }", R"({ "name": "ok", "value": 42 })");
  assert_xeq("x = { 1, 2, 3, extra = true }", R"([ 1, 2, 3, { "extra": true } ])");
  assert_xeq("x = { extra = true, 1, 2, 3 }", R"([ 1, 2, 3, { "extra": true } ])");
  assert_xeq("x = {}", "[]");
}
