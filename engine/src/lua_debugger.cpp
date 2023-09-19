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
/**
 * \file stack_lua.cpp
 */

#include "lua_setup.hpp"

#include <lrdb/server.hpp>  // lrdb::server
#include <sol/state_view.hpp>  // for state_view

namespace cloe {

void start_lua_debugger(sol::state& lua, int listen_port) {
  static lrdb::server debug_server(listen_port);
  debug_server.reset(lua.lua_state());
}

}  // namespace cloe
