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

#include <fable/utility/sol.hpp>

#include "lua_api.hpp"
#include "lua_setup.hpp"
#include "stack.hpp"

namespace cloe {

void register_usertype_stack(sol::table& lua) {
  auto stack = lua.new_usertype<Stack>("Stack", sol::no_constructor);
  stack["merge_stackfile"] = &Stack::merge_stackfile;
  stack["merge_stackjson"] = [](Stack& self, const std::string& json, std::string file) {
    self.from_conf(Conf{fable::parse_json(json), std::move(file)});
  };
  stack["merge_stacktable"] = [](Stack& self, sol::object obj, std::string file) {
    self.from_conf(Conf{Json(obj), std::move(file)});
  };
}

}  // namespace cloe
