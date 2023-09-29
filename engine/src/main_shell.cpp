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
#include <fable/utility/string.hpp>  // for ends_with

#include "lua_api.hpp"        // for lua_safe_script_file
#include "main_commands.hpp"  // for Stack, new_stack, LuaOptions, new_lua
#include "stack.hpp"          // for Stack

namespace engine {

template <typename S>
void print_error(std::ostream& os, const S& chunk) {
  auto err = sol::error(chunk);
  os << sol::to_string(chunk.status()) << " error: " << err.what() << std::endl;
}

bool evaluate(sol::state& lua, std::ostream& os, const char* buf) {
  try {
    auto result = lua.safe_script(buf, sol::script_pass_on_error);
    if (!result.valid()) {
      print_error(os, result);
      return false;
    }
  } catch (const std::exception& e) {
    os << "runtime error: " << e.what() << std::endl;
    return false;
  }
  return true;
}

int noninteractive_shell(sol::state& lua, std::ostream& os, const std::vector<std::string>& actions,
                         bool ignore_errors) {
  int errors = 0;
  for (const auto& action : actions) {
    auto ok = evaluate(lua, os, action.c_str());
    if (!ok) {
      errors++;
      if (!ignore_errors) {
        break;
      }
    }
  }
  return errors;
}

void interactive_shell(sol::state& lua, std::ostream& os, const std::vector<std::string>& actions,
                       bool ignore_errors) {
  constexpr auto PROMPT = "> ";
  constexpr auto PROMPT_CONTINUE = ">> ";
  constexpr auto PROMPT_HISTORY = "< ";
  constexpr auto HISTORY_LENGTH = 1024;

  // Set up linenoise library
  linenoiseSetMultiLine(1);
  linenoiseHistorySetMaxLen(HISTORY_LENGTH);

  os << "Cloe " << CLOE_ENGINE_VERSION << " Lua interactive shell" << std::endl;
  os << "Press [Ctrl+D] or [Ctrl+C] to exit." << std::endl;

  // Run actions from command line first
  auto remaining = actions.size();
  for (const auto& action : actions) {
    os << PROMPT_HISTORY << action << std::endl;
    linenoiseHistoryAdd(action.c_str());
    remaining--;
    auto ok = evaluate(lua, os, action.c_str());
    if (!ok && !ignore_errors) {
      break;
    }
  }
  if (remaining != 0) {
    os << "warning: dropping to interactive console early due to error" << std::endl;
  }

  // Start REPL loop
  std::string buf;
  std::string vbuf;
  for (;;) {
    auto* line = linenoise(buf.empty() ? PROMPT : PROMPT_CONTINUE);
    if (line == nullptr) {
      break;
    }
    buf += line;
    linenoiseFree(line);


    sol::load_result chunk;
    {
      // Enable return value printing by injecting "return";
      // if it does not parse, then we abort and use original value.
      vbuf = "return " + buf;
      chunk = lua.load(vbuf);
      if (!chunk.valid()) {
        chunk = lua.load(buf);
      }
    }

    if (!chunk.valid()) {
      auto err = sol::error(chunk);
      if (fable::ends_with(err.what(), "near <eof>")) {
        // The following error messages seem to indicate
        // that Lua is just waiting for more to complete the statement:
        //
        //     'end' expected near <eof>
        //     unexpected symbol near <eof>
        //     <name> or '...' expected near <eof>
        //
        // In this case, we don't clear buf, but instead allow
        // the user to continue inputting on the next line.
        buf += " ";
        continue;
      }
      print_error(os, chunk);
      buf.clear();
      continue;
    }

    auto script = chunk.get<sol::protected_function>();
    auto result = script();
    if (!result.valid()) {
      print_error(os, result);
    } else if (result.return_count() > 0) {
      for (auto r : result) {
        lua["cloe"]["describe"](r);
      }
    }

    // Clear buf for next input line
    linenoiseHistoryAdd(buf.c_str());
    buf.clear();
  }
}

int shell(const ShellOptions& opt, const std::vector<std::string>& filepaths) {
  assert(opt.output != nullptr && opt.error != nullptr);

  cloe::StackOptions stack_opt = opt.stack_options;
  cloe::Stack stack = cloe::new_stack(stack_opt);
  sol::state lua = cloe::new_lua(opt.lua_options, stack);

  // Collect input files and strings to execute
  std::vector<std::string> actions{};
  actions.reserve(filepaths.size() + opt.commands.size());
  for (const auto& file : filepaths) {
    actions.emplace_back(fmt::format("dofile(\"{}\")", file));
  }
  actions.insert(actions.end(), opt.commands.begin(), opt.commands.end());

  // Determine whether we should be interactive or not
  bool interactive = opt.interactive ? *opt.interactive : opt.commands.empty() && filepaths.empty();
  if (!interactive) {
    auto errors = noninteractive_shell(lua, *opt.error, actions, opt.ignore_errors);
    if (errors != 0) {
      return EXIT_FAILURE;
    }
  } else {
    interactive_shell(lua, *opt.output, actions, opt.ignore_errors);
  }
  return EXIT_SUCCESS;
}

}  // namespace engine
