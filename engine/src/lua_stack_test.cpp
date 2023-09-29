
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <sol/state.hpp>

#include <cloe/core.hpp>                     // for Json
#include <fable/utility/gtest.hpp>           // for assert_from_conf
#include <fable/utility/sol.hpp>
#include <fable/utility.hpp>

#include "stack.hpp"                         // for Stack
#include "lua_setup.hpp"
using namespace cloe;                        // NOLINT(build/namespaces)

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
