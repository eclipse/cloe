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

#include <iostream>  // for ostream, cout

#include <cloe/core/fable.hpp>
#include <cloe/plugin.hpp>        // for CLOE_PLUGIN_MANIFEST_VERSION
#include <cloe/utility/inja.hpp>  // for inja_env

#include "main_commands.hpp" // for VersionOptions
#include "config.hpp"  // for CLOE_STACK_VERSION

namespace engine {

int version(const VersionOptions& opt) {
  cloe::Json v{
      {"engine", CLOE_ENGINE_VERSION},                             // from CMakeLists.txt
      {"build_date", CLOE_ENGINE_TIMESTAMP},                       // from CMakeLists.txt
      {"stack", CLOE_STACK_VERSION},                               // from "stack.hpp"
      {"plugin_manifest", CLOE_PLUGIN_MANIFEST_VERSION},           // from <cloe/plugin.hpp>
      {"feature_server", CLOE_ENGINE_WITH_SERVER ? true : false},  // from CMakeLists.txt
  };

  if (opt.output_json) {
    *opt.output << v.dump(opt.json_indent) << std::endl;
  } else {
    auto env = cloe::utility::inja_env();
    *opt.output << env.render(R"(Cloe [[engine]]

Engine Version:  [[engine]]
Build Date:      [[build_date]]
Stack:           [[stack]]
Plugin Manifest: [[plugin_manifest]]
Features:
  server: [[feature_server]]
)",
                             v);
  }

  return EXIT_SUCCESS;
}

}  // namespace engine
