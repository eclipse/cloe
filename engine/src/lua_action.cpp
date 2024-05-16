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
#include <utility>

#include <cloe/sync.hpp>
#include <cloe/trigger.hpp>

#include "lua_api.hpp"

namespace engine {
namespace actions {

cloe::CallbackResult LuaFunction::operator()(const cloe::Sync& sync, cloe::TriggerRegistrar&) {
  logger()->trace("Running lua function.");
  auto result = func_(std::ref(sync));
  if (!result.valid()) {
    throw cloe::Error("error executing Lua function: {}", sol::error{result}.what());
  }
  // Return false from a pinned action to remove it.
  if (result.return_count() > 0 && !result.get<bool>()) {
    return cloe::CallbackResult::Unpin;
  }
  return cloe::CallbackResult::Ok;
}

cloe::CallbackResult Lua::operator()(const cloe::Sync&, cloe::TriggerRegistrar&) {
  logger()->trace("Running lua script.");
  auto result = lua_.script(script_);
  if (!result.valid()) {
    throw cloe::Error("error executing Lua function: {}", sol::error{result}.what());
  }
  // Return false from a pinned action to remove it.
  if (result.return_count() > 0 && !result.get<bool>()) {
    return cloe::CallbackResult::Unpin;
  }
  return cloe::CallbackResult::Ok;
}

void Lua::to_json(cloe::Json& j) const {
  j = cloe::Json{
      {"script", script_},
  };
}

cloe::TriggerSchema LuaFactory::schema() const {
  static const char* desc = "lua script to execute";
  return cloe::TriggerSchema{
      this->name(), this->description(), cloe::InlineSchema(desc, cloe::JsonType::string, true),
      cloe::Schema{
          {"script", cloe::make_prototype<std::string>("lua script to execute")},
      }};
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
