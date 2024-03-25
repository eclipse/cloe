#include <cloe/simulation/registrar.hpp>
#include "cloe/lua/lua_simulation_driver.hpp"

namespace engine {

cloe::LuaSimulationDriver* Registrar::lua_simulation_driver() {
  return dynamic_cast<cloe::LuaSimulationDriver*>(driver_);
}

}

