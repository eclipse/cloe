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

TEST(fable_utility_sol, from_json_bool) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  lua["json"] = fable::into_sol_object(lua, Json(true));
  lua.script(
  R"(
    assert(type(json) == "boolean")
    assert(json == true)
  )");
}

TEST(fable_utility_sol, from_json_int) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  lua["json"] = fable::into_sol_object(lua, Json(42));
  lua.script(
  R"(
    assert(type(json) == "number")
    assert(json == 42)
  )");
}

TEST(fable_utility_sol, from_json_float) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  lua["json"] = fable::into_sol_object(lua, Json(3.14159));
  lua.script(
  R"(
    assert(type(json) == "number")
    assert(json == 3.14159)
  )");
}

TEST(fable_utility_sol, from_json_string) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  lua["json"] = fable::into_sol_object(lua, Json("hello world!"));
  lua.script(
  R"(
    assert(type(json) == "string")
    assert(json == "hello world!")
  )");
}

TEST(fable_utility_sol, from_json_array) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  lua["json"] = fable::into_sol_object(lua, Json({1, 2, 3}));
  lua.script(
  R"(
    assert(type(json) == "table")
    assert(#json == 3)
    assert(json[1] == 1)
    assert(json[2] == 2)
    assert(json[3] == 3)
  )");
}

TEST(fable_utility_sol, from_json_object) {
  auto lua = sol::state();
  lua.open_libraries(sol::lib::base);

  auto json = Json{
    {"xbool", true},
    {"xstring", "hello world!"},
    {"xint", -42}, // NOLINT
    {"xunsigned", 42u}, // NOLINT
    {"xdouble", 42.0}, // NOLINT
    {"xarray", {1, 2, 3}},
    {"xobject", {
      {"foo", "bar"},
    }},
    {"xnull", nullptr},
  };

  auto tmp = sol::object(lua["json"]);
  nlohmann::adl_serializer<sol::object>::from_json(json, tmp);
  lua["json"] = tmp;
  lua.script(
  R"(
    print(json)
    assert(json)

    print(json.xbool)
    assert(json.xbool == true)

    print(json.xstring)
    assert(json.xstring == "hello world!")

    print(json.xint)
    assert(json.xint == -42)

    print(json.xunsigned)
    assert(json.xunsigned == 42)

    print(json.xdouble)
    assert(json.xdouble == 42.0)

    assert(type(json.xarray) == "table")
    assert(json.xarray[1] == 1)
    assert(json.xarray[3] == 3)
    assert(json.xarray[4] == nil)

    print(json.xobject)
    assert(json.xobject)
    assert(json.xobject.foo == "bar")

    assert(not json.xnull)
  )");
}
