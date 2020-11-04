/*
 * Copyright 2020 Robert Bosch GmbH
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
 * \file stack_migrations.cpp
 */

#include <map>
#include <string>

#include "stack.hpp"

namespace cloe {

using MigrateFunc = std::function<Conf(Conf)>;

std::map<std::string, MigrateFunc> MIGRATIONS{
    {"3",
     [](Conf c) -> Conf {
       if (c.has("app")) {
         c.move("app", "engine");
       }
       c.set("version", CLOE_STACK_VERSION);
       return c;
     }},
};

}  // namespace cloe
