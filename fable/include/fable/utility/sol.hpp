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
 * \file fable/utility/sol.hpp
 *
 * This file contains specializations of `nlohmann::adl_serializer` for sol2
 * types.
 *
 * In order to provide serialization for third-party types, we need to either
 * use their namespace or provide a specialization in that of nlohmann. It is
 * illegal to define anything in the std namespace, so we are left no choice in
 * this regard.
 *
 * See: https://github.com/nlohmann/json
 */

#pragma once

#include <nlohmann/json.hpp>

#include <sol/object.hpp>    // for object
#include <sol/optional.hpp>  // for optional
#include <sol/table.hpp>     // for table
#include <sol/version.hpp>   // for SOL_IS_OFF, SOL_SAFE_NUMERICS, ...

#if SOL_IS_OFF(SOL_SAFE_NUMERICS) && SOL_IS_OFF(SOL_ALL_SAFETIES_ON)
#error "nlohmann support for sol requires at least SOL_SAFE_NUMERICS=1"
#include <force_compiler_to_stop_here>
#endif

namespace nlohmann {

/**
 * Unfortunately, the way that sol is implemented, when json j = lua["key"],
 * lua["key"].operator json&() is called, which leads to an error if lua["key"]
 * is anything other than the usertype json.
 *
 * For this reason, it's required to wrap any table access with something
 * that turns it into a sol::object:
 *
 *    json j1 = lua["key"].template get<sol::object>();
 *    json j2 = sol::object(lua["key"]);
 */
template <>
struct adl_serializer<sol::object> {
  static void to_json_array(json& j, const sol::table& tbl) {
    if (j.type() != json::value_t::array) {
      j = json::array();
    }
    auto kv_args = json::object();
    for (auto& kv : tbl) {
      if (kv.first.get_type() != sol::type::number) {
        auto key = kv.first.as<std::string>();
        to_json(kv_args[key], kv.second.as<sol::object>());
        continue;
      }
      j.emplace_back(kv.second.as<sol::object>());
    }
    if (!kv_args.empty()) {
      j.emplace_back(std::move(kv_args));
    }
  }

  static void to_json_object(json& j, const sol::table& tbl) {
    if (j.type() != json::value_t::object) {
      j = json::object();
    }
    for (auto& kv : tbl) {
      auto key = kv.first.as<std::string>();
      to_json(j[key], kv.second.as<sol::object>());
    }
  }

  static void to_json(json& j, const sol::table& tbl) {
    if (tbl.pairs().begin() == tbl.pairs().end()) {
      // We don't know whether this is an empty array or an empty object,
      // but it's probably an array since this makes more sense to have.
      if (j.type() == json::value_t::null) {
        j = json::array();
      }
      return;
    }

    // Lua only accepts table keys that are integers or strings,
    // but they can be mixed; we use the first element to guess
    // whether its an array or an object.
    auto first = (*tbl.pairs().begin()).first;
    bool looks_like_array = (first.get_type() == sol::type::number);
    if (looks_like_array) {
      to_json_array(j, tbl);
    } else {
      to_json_object(j, tbl);
    }
  }

  static void to_json(json& j, const sol::object& obj) {
    switch (obj.get_type()) {
      case sol::type::table: {
        to_json(j, obj.as<sol::table>());
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
        // If the number in Lua has any significant decimals, even if they are zero,
        // it is not an integer and this optional will be falsy.
        if (auto num = obj.as<sol::optional<int64_t>>(); num) {
          j = *num;
        } else {
          j = obj.as<double>();
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
        j = "<userdata>";
        break;
    }
  }
};

}  // namespace nlohmann
