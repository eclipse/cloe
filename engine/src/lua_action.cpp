/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file lua_action.cpp
 * \see  lua_action.hpp
 */

#include "lua_action.hpp"

#include <string>
#include <optional>
#include <utility>

#include <sol/optional.hpp>

#include <cloe/sync.hpp>
#include <cloe/trigger.hpp>

#include "lua_api.hpp"

namespace engine {

cloe::TriggerPtr make_trigger_from_lua(cloe::TriggerRegistrar& r, const sol::table& lua) {
  sol::optional<std::string> label = lua["label"];

  cloe::EventPtr event = r.make_event(cloe::Conf{cloe::Json(lua["event"])});

  cloe::ActionPtr action;
  if (lua["action"].get_type() == sol::type::function) {
    action = std::make_unique<actions::LuaFunction>("luafunction", lua["action"]);
    if (!label) {
      label = lua["action_source"];
    }
  } else {
    action = r.make_action(cloe::Conf{cloe::Json(lua["action"])});
  }

  sol::optional<bool> sticky = lua["sticky"];

  auto trigger = std::make_unique<cloe::Trigger>(label.value_or(""), cloe::Source::LUA,
                                                 std::move(event), std::move(action));
  trigger->set_sticky(sticky.value_or(false));
  return trigger;
}

namespace actions {

void Lua::operator()(const cloe::Sync&, cloe::TriggerRegistrar&) {
  auto result = lua_.script(script_);
  if (!result.valid()) {
    sol::error err = result;
    throw err;
  }
}

void Lua::to_json(cloe::Json& j) const {
  j = cloe::Json{
    {"script", script_},
  };
}

cloe::TriggerSchema LuaFactory::schema() const {
  static const char* desc = "lua script to execute";
  return cloe::TriggerSchema{
      this->name(),
      this->description(),
      cloe::InlineSchema(desc, cloe::JsonType::string, true),
      cloe::Schema{
        {"script", cloe::make_prototype<std::string>("lua script to execute")},
      }
  };
}

cloe::ActionPtr LuaFactory::make(const cloe::Conf& c) const {
  auto script = c.get<std::string>("script");
  return std::make_unique<Lua>(name(), script, lua_);
}

cloe::ActionPtr LuaFactory::make(const std::string& s) const {
  return make(cloe::Conf{cloe::Json{
      {"script", s},
  }});
}

}  // namespace actions
}  // namespace engine
