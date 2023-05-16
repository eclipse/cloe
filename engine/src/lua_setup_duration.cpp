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

#include "lua_setup.hpp"

#include <cloe/core/duration.hpp>

namespace cloe {

void register_usertype_duration(sol::table& lua) {
  Duration (*parse_duration_ptr)(const std::string&) = ::cloe::parse_duration;
  std::string (*to_string_ptr)(const Duration&) = ::cloe::to_string;
  lua.new_usertype<::cloe::Duration>("Duration",
    sol::factories(parse_duration_ptr),
    sol::meta_function::to_string, to_string_ptr,
    sol::meta_function::addition,
    sol::resolve<Duration(const Duration&, const Duration&)>(std::chrono::operator+),
    sol::meta_function::subtraction,
    sol::resolve<Duration(const Duration&, const Duration&)>(std::chrono::operator-),
    sol::meta_function::division,
    [](const Duration& x, double d) -> Duration { Duration y(x); y /= d; return y; },
    sol::meta_function::multiplication,
    [](const Duration& x, double d) -> Duration { Duration y(x); y *= d; return y; },
    "ns", &Duration::count,
    "us", [](const Duration& d) -> double { return static_cast<double>(d.count()) / 10e2; },
    "ms", [](const Duration& d) -> double { return static_cast<double>(d.count()) / 10e5; },
    "s", [](const Duration& d) -> double { return static_cast<double>(d.count()) / 10e8; }
  );
}

}  // namespace cloe
