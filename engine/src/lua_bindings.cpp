#include "lua_bindings.hpp"
#include <cloe/coordinator.hpp>
#include "lua_simulation_driver.hpp"

namespace engine{
void register_usertype_coordinator(sol::table& lua, const cloe::Sync& sync){
  // clang-format off
  lua.new_usertype<cloe::coordinator::Coordinator>("Coordinator",
    sol::no_constructor,
    "insert_trigger", [&sync](cloe::coordinator::Coordinator& self, const sol::object& obj) {
      self.insert_trigger(sync, engine::LuaSimulationDriver::make_trigger(self.trigger_factory(), obj));
    },
    "execute_action", [&sync](cloe::coordinator::Coordinator& self, const sol::object& obj) {
      auto action = engine::LuaSimulationDriver::make_action(self.trigger_factory(), obj);
      self.execute_action(sync, *action);
    }
  );
  // clang-format on
}
}
