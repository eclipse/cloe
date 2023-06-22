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

#include "lua_api.hpp"

#include <filesystem>  // for path
#include <stdexcept>   // for exception

#include <nlohmann/json.hpp>   // for json
#include <sol/object.hpp>      // for object
#include <sol/state_view.hpp>  // for state_view
#include <cloe/core/logger.hpp>

namespace cloe {

sol::protected_function_result lua_safe_script_file(sol::state_view& lua,
                                                    const std::filesystem::path& filepath) {
  auto file = std::filesystem::path(filepath);
  auto dir = file.parent_path().generic_string();
  if (dir == "") {
    dir = ".";
  }

  auto api = lua["cloe"]["api"];
  api["THIS_SCRIPT_FILE"] = file.generic_string();
  api["THIS_SCRIPT_DIR"] = dir;
  logger::get("cloe")->info("Loading {}", file.generic_string());
  auto result = lua.safe_script_file(file.generic_string(), sol::script_pass_on_error);
  api["THIS_SCRIPT_FILE"] = nullptr;
  api["THIS_SCRIPT_DIR"] = nullptr;
  return result;
}

// nlohmann::json to_json(const sol::object& obj) {
//   nlohmann::json j;
//   nlohmann::to_json(j, obj);
//   return j;
// }

}  // namespace cloe

/*
 * In order to provide serialization for third-party types, we need to either
 * use their namespace or provide a specialization in that of nlohmann. It is
 * illegal to define anything in the std namespace, so we are left no choice in
 * this regard.
 *
 * See: https://github.com/nlohmann/json
 */
namespace nlohmann {

void adl_serializer<sol::object>::to_json(json& j, const sol::object& obj) {
  switch (obj.get_type()) {
    case sol::type::table: {
      for (auto& kv : obj.as<sol::table>()) {
        auto key = kv.first.as<std::string>();
        j[key] = json();
        const sol::object& val = kv.second.as<sol::object>();
        to_json(j[key], val);
      }
      break;
    }
    case sol::type::string: {
      j = obj.as<std::string>();
      break;
    }
    case sol::type::boolean: {
      j = obj.as<bool>();
      break;
    }
    case sol::type::number: {
      std::optional<double> dbl = obj.as<std::optional<double>>();
      if (dbl) {
        j = *dbl;
      } else {
        j = obj.as<int64_t>();
      }
      break;
    }
    case sol::type::nil:
    case sol::type::none: {
      j = nullptr;
      break;
    }
    case sol::type::poly:
      // throw std::out_of_range("cannot serialize lua poly type to JSON");
      j = "<poly>";
      break;
    case sol::type::function:
      // throw std::out_of_range("cannot serialize lua function to JSON");
      j = "<function>";
      break;
    case sol::type::thread:
      // throw std::out_of_range("cannot serialize lua thread to JSON");
      j = "<thread>";
      break;
    case sol::type::userdata:
    case sol::type::lightuserdata:
      // TODO: If we want to serialize userdata, we need to extend this here.
      j = "<userdata>";
      break;
  }
}

}  // namespace nlohmann
