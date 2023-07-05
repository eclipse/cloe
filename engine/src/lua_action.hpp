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
 * \file lua_action.hpp
 * \see  lua_action.cpp
 *
 * This file contains several types that make use of cloe::Lua.
 */

#pragma once

#include <sol/state_view.hpp>
#include <sol/function.hpp>

#include <cloe/core.hpp>     // for Logger, Json, Conf, ...
#include <cloe/trigger.hpp>  // for Action, ActionFactory, ...

namespace engine {

cloe::TriggerPtr make_trigger_from_lua(cloe::TriggerRegistrar& r, const sol::table& lua);

namespace actions {

class LuaFunction : public cloe::Action {
 public:
  LuaFunction(const std::string& name, sol::function fun) : Action(name), func_(fun) {}

  cloe::ActionPtr clone() const override {
    return std::make_unique<LuaFunction>(this->name(), func_);
  }

  void operator()(const cloe::Sync& sync, cloe::TriggerRegistrar&) override {
    logger()->trace("Running lua function.");
    auto result = func_(std::ref(sync));
    if(!result.valid()) {
      throw cloe::Error("error executing Lua function: {}", sol::error{result}.what());
    }
  }

  void to_json(cloe::Json& j) const override { j = cloe::Json{}; }

 private:
  sol::protected_function func_;
};

class Lua : public cloe::Action {
 public:
  Lua(const std::string& name, const std::string& script, sol::state_view lua)
      : Action(name), script_(script), lua_(lua) {}

  cloe::ActionPtr clone() const override { return std::make_unique<Lua>(name(), script_, lua_); }

  void operator()(const cloe::Sync&, cloe::TriggerRegistrar&) override;

 protected:
  void to_json(cloe::Json& j) const override;

 private:
  std::string script_;
  sol::state_view lua_;
};

class LuaFactory : public cloe::ActionFactory {
 public:
  using ActionType = Lua;
  explicit LuaFactory(sol::state_view lua)
      : cloe::ActionFactory("lua", "run a lua script"), lua_(lua) {
  }
  cloe::TriggerSchema schema() const override;
  cloe::ActionPtr make(const cloe::Conf& c) const override;
  cloe::ActionPtr make(const std::string& s) const override;

 private:
  sol::state_view lua_;
};

}  // namespace actions
}  // namespace engine
