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

#include <iostream>  // for cout, cerr, endl
#include <optional>  // for optional<>
#include <string>    // for string
#include <utility>   // for make_pair<>
#include <vector>    // for vector<>

#include <linenoise.h>

#include <fmt/format.h>

#include "lua_api.hpp"     // for lua_safe_script_file
#include "stack.hpp"           // for Stack
#include "main_commands.hpp"   // for Stack, new_stack, LuaOptions, new_lua

namespace engine {

int shell(const ShellOptions& opt, const std::vector<std::string>& filepaths) {
  assert(opt.output != nullptr && opt.error != nullptr);

  cloe::StackOptions stack_opt = opt.stack_options;
  cloe::Stack stack = cloe::new_stack(stack_opt);
  sol::state lua = cloe::new_lua(opt.lua_options, stack);

  // Determine whether we should be interactive or not
  bool interactive = opt.interactive ? *opt.interactive : opt.commands.empty() && filepaths.empty();
  if (interactive) {
    linenoiseSetMultiLine(1);
    linenoiseHistorySetMaxLen(1024);
  }

  auto evaluate = [&](const char* buf) {
    try {
      auto result = lua.safe_script(buf, sol::script_pass_on_error);
      if (!result.valid()) {
        sol::error err = result;
        *opt.error << "Error: " << err.what() << std::endl;
        return false;
      }
    } catch (const std::exception& e) {
      *opt.error << "Error: " << e.what() << std::endl;
      return false;
    }
    return true;
  };

  auto evaluate_file = [&](const std::string& file) {
    try {
      auto result = cloe::lua_safe_script_file(lua, file);
      if (!result.valid()) {
        sol::error err = result;
        *opt.error << "Error: " << err.what() << std::endl;
        return false;
      }
    } catch (const std::exception& e) {
      *opt.error << "Error: " << e.what() << std::endl;
      return false;
    }
    return true;
  };

  std::vector<std::pair<std::string, std::function<bool()>>> actions{};
  for (const auto& file : filepaths) {
    auto cmd = fmt::format("dofile(\"{}\")", file);
    actions.emplace_back(std::make_pair(std::move(cmd), [&]() { return evaluate_file(file); }));
  }
  for (const auto& cmd : opt.commands) {
    actions.emplace_back(std::make_pair(cmd, [&]() { return evaluate(cmd.c_str()); }));
  }

  if (interactive) {
    *opt.output << "Cloe " << CLOE_ENGINE_VERSION << " Lua interactive shell" << std::endl;
    *opt.output << "Press [Ctrl+D] or [Ctrl+C] to exit." << std::endl;
  }
  for (size_t i = 0; i < actions.size(); i++) {
    auto action = actions[i];
    if (interactive) {
      *opt.output << "<<< " << action.first << std::endl;
      linenoiseHistoryAdd(action.first.c_str());
    }
    auto ok = action.second();
    if (!ok) {
      if (opt.ignore_errors) {
        continue;
      }
      if (!interactive) {
        return EXIT_FAILURE;
      }
      if (i < actions.size() - 1) {
        *opt.error << "Warning: dropping to interactive console early due to error." << std::endl;
      }
      break;
    }
  }

  if (!interactive) {
    return EXIT_SUCCESS;
  }

  for (;;) {
    char* buf = linenoise(">>> ");
    if (buf == nullptr) {
      break;
    }
    linenoiseHistoryAdd(buf);
    evaluate(buf);
    free(buf);
  }

  return EXIT_SUCCESS;
}

}  // namespace engine
