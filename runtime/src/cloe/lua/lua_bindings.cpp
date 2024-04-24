#include <cloe/lua/lua_bindings.hpp>
#include <cloe/lua/lua_simulation_driver.hpp>
#include <cloe/coordinator.hpp>

namespace engine{
void register_usertype_coordinator(sol::table& lua, const cloe::Sync& sync){
  // clang-format off
  lua.new_usertype<cloe::coordinator::Coordinator>("Coordinator",
    sol::no_constructor,
    "insert_trigger", [&sync](cloe::coordinator::Coordinator& self, const sol::object& obj) {
      self.insert_trigger(sync, cloe::LuaSimulationDriver::make_trigger(self.trigger_factory(), obj));
    },
    "execute_action", [&sync](cloe::coordinator::Coordinator& self, const sol::object& obj) {
      auto action = cloe::LuaSimulationDriver::make_action(self.trigger_factory(), obj);
      self.execute_action(sync, *action);
    }
  );
  // clang-format on
}
}
