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
#include <string>
#include <vector>

#include <sol/state.hpp>

#include <cloe/core.hpp>   // for Json
#include <cloe/stack.hpp>  // for Stack
#include <fable/utility.hpp>
#include <fable/utility/gtest.hpp>  // for assert_from_conf
#include <fable/utility/sol.hpp>

#include "lua_setup.hpp"
using namespace cloe;  // NOLINT(build/namespaces)

TEST(cloe_lua_stack, deserialize_vehicle_conf) {
  sol::state lua;

  lua.open_libraries(sol::lib::base);

  lua.script("from = { index = 0, simulator = \"nop\" }");
  lua.script("print(from.index)");

  cloe::FromSimulator fromsim;
  sol::object obj = lua["from"];
  cloe::Json json(obj);
  try {
    fromsim.from_conf(Conf{json});
  } catch (fable::SchemaError& err) {
    fable::pretty_print(err, std::cerr);
    FAIL();
  }
}

TEST(cloe_lua_stack, convert_json_lua) {
  sol::state lua;
  lua.open_libraries(sol::lib::base);

  Stack s;

  lua["stack"] = fable::into_sol_object(lua, s.active_config());
  lua.script(R"(
    assert(stack)
    assert(stack.version == "4.1")
    assert(stack.engine)
  )");
}

TEST(cloe_lua_stack, copy_stack_json_lua) {
  sol::state lua;
  lua.open_libraries(sol::lib::base);

  Stack s1;
  s1.engine.keep_alive = true;  // change something
  Stack s2 = s1;                // copy

  lua["s1"] = fable::into_sol_object(lua, s1.active_config());
  lua["s2"] = fable::into_sol_object(lua, s2.active_config());
  lua.script(R"(
    assert(s1)
    assert(s1.version == "4.1")
    assert(s1.engine)
  )");

  lua.script(R"(
    function deep_equal(a, b)
      if a == b then
        return true
      end
      if type(a) ~= type(b) then
        return false
      end
      if type(a) == 'table' then
        for k, v in pairs(a) do
          if not deep_equal(v, b[k]) then
            return false
          end
        end
        for k, _ in pairs(b) do
          if a[k] == nil then
            return false
          end
        end
        return true
      end
      return false
    end
    assert(deep_equal(s1, s2))
  )");
}
