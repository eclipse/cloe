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

#include <cloe/sync.hpp>

namespace cloe {

void register_usertype_sync(sol::table& lua) {
  lua.new_usertype<::cloe::Sync>
  ("Sync",
    sol::no_constructor,
    "step", &Sync::step,
    "step_width", &Sync::step_width,
    "time", &Sync::time,
    "eta", &Sync::eta,
    "realtime_factor", &Sync::realtime_factor,
    "is_realtime_factor_unlimited", &Sync::is_realtime_factor_unlimited,
    "achievable_realtime_factor", &Sync::achievable_realtime_factor
  );
}

}  // namespace cloe
