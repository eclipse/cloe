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
 * \file main_commands.hpp
 * \see  main.cpp
 */

#include <optional>
#include <string>
#include <vector>

#include <cloe/stack_config.hpp>
#include <cloe/stack_factory.hpp>

#include "lua_setup.hpp"

namespace engine {

struct CheckOptions {
  cloe::StackOptions stack_options;
  cloe::LuaOptions lua_options;

  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;
  std::string delimiter = ",";

  // Flags:
  bool summarize = false;
  bool output_json = false;
  int json_indent = 2;
};

int check(const CheckOptions& opt, const std::vector<std::string>& filepaths);

struct DumpOptions {
  cloe::StackOptions stack_options;
  cloe::LuaOptions lua_options;

  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;

  // Flags:
  int json_indent = 2;
};

int dump(const DumpOptions& opt, const std::vector<std::string>& filepaths);

struct ProbeOptions {
  cloe::StackOptions stack_options;
  cloe::LuaOptions lua_options;

  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;

  // Options
  std::string uuid;  // Not currently used.

  // Flags:
  int json_indent = 2;

  bool debug_lua = false;                       // Not currently used.
  int debug_lua_port = CLOE_LUA_DEBUGGER_PORT;  // Not currently used.
};

int probe(const ProbeOptions& opt, const std::vector<std::string>& filepaths);

struct RunOptions {
  cloe::StackOptions stack_options;
  cloe::LuaOptions lua_options;

  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;

  // Options
  std::string uuid;
  std::string output_path;

  // Flags:
  int json_indent = 2;
  bool allow_empty = false;
  bool write_output = true;
  bool require_success = false;
  bool report_progress = true;

  bool debug_lua = false;
  int debug_lua_port = CLOE_LUA_DEBUGGER_PORT;
};

int run(const RunOptions& opt, const std::vector<std::string>& filepaths);

struct ShellOptions {
  cloe::StackOptions stack_options;
  cloe::LuaOptions lua_options;

  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;

  // Options:
  std::vector<std::string> commands;

  // Flags:
  std::optional<bool> interactive;
  bool ignore_errors;
};

int shell(const ShellOptions& opt, const std::vector<std::string>& filepaths);

struct UsageOptions {
  cloe::StackOptions stack_options;
  cloe::LuaOptions lua_options;

  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;

  // Flags:
  bool plugin_usage = false;
  bool output_json = false;
  int json_indent = 2;
};

int usage(const UsageOptions& opt, const std::string& argument);

struct VersionOptions {
  std::ostream* output = &std::cout;
  std::ostream* error = &std::cerr;

  // Flags:
  bool output_json = false;
  int json_indent = 2;
};

int version(const VersionOptions& opt);

// ------------------------------------------------------------------------- //

class Simulation;

extern Simulation* GLOBAL_SIMULATION_INSTANCE;  // NOLINT

/**
 * Handle interrupt signals sent by the operating system.
 *
 * When this function is called, it cannot call any other functions that
 * might have set any locks, because it might not get the lock, and then the
 * program hangs instead of gracefully exiting. It's a bit sad, true, but
 * that's the way it is.
 *
 * That is why you cannot make use of the logging in this function. You also
 * cannot make use of triggers, because they also have a lock.
 *
 * The function immediately resets the signal handler to the default provided
 * by the standard library, so that in the case that we do hang for some
 * reasons, the user can force abort by sending the signal a third time.
 */
void handle_signal(int sig);

}  // namespace engine
