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
 * \file utility/lua_action.hpp
 * \see  utility/lua_action.cpp
 *
 * This file contains several types that make use of cloe::Lua.
 */

#pragma once

#include <sol/sol.hpp>

#include <cloe/core.hpp>             // for Logger, Json, Conf, ...
#include <cloe/trigger.hpp>          // for Action, ActionFactory, ...

namespace engine {
namespace actions {

class Lua : public cloe::Action {
 public:
  Lua(const std::string& name, const std::string& script, sol::state* state)
      : Action(name), script_(script), state_(state) {
    assert(state_ != nullptr);
  }

  cloe::ActionPtr clone() const override {
    return std::make_unique<Lua>(name(), script_, state_);
  }

  void operator()(const cloe::Sync&, cloe::TriggerRegistrar&) override;

 protected:
  void to_json(cloe::Json& j) const override;

 private:
  std::string script_;
  sol::state* state_{nullptr};
};

class LuaFactory : public cloe::ActionFactory {
 public:
  using ActionType = Lua;
  explicit LuaFactory(sol::state* state)
      : cloe::ActionFactory("lua", "run a lua script"), state_(state) {
    assert(state_ != nullptr);
  }
  cloe::TriggerSchema schema() const override;
  cloe::ActionPtr make(const cloe::Conf& c) const override;
  cloe::ActionPtr make(const std::string& s) const override;

 private:
  sol::state* state_{nullptr};
};

}  // namespace actions
}  // namespace engine
