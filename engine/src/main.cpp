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

#include <iostream>  // for cerr
#include <string>    // for string
#include <utility>   // for swap

#include <CLI/CLI.hpp>

#include <cloe/core/error.hpp>
#include <cloe/core/logger.hpp>
#include <cloe/stack_config.hpp>

#include "main_commands.hpp"

int main(int argc, char** argv) {
  CLI::App app("Cloe " CLOE_ENGINE_VERSION);
  app.option_defaults()->always_capture_default();

  // Version Command:
  engine::VersionOptions version_options{};
  auto* version = app.add_subcommand("version", "Show program version information.");
  version->add_flag("-j,--json", version_options.output_json,
                    "Output version information as JSON data");
  version->add_option("-J,--json-indent", version_options.json_indent, "JSON indentation level");

  // Usage Command:
  engine::UsageOptions usage_options{};
  std::string usage_key_or_path;
  auto* usage = app.add_subcommand("usage", "Show schema or plugin usage information.");
  usage->add_flag("-j,--json", usage_options.output_json, "Output global/plugin JSON schema");
  usage->add_option("-J,--json-indent", usage_options.json_indent, "JSON indentation level");
  usage->add_option("files", usage_key_or_path, "Plugin name, key or path to show schema of");

  // Dump Command:
  engine::DumpOptions dump_options{};
  std::vector<std::string> dump_files;
  auto* dump = app.add_subcommand("dump", "Dump configuration of (merged) stack files.");
  dump->add_option("-J,--json-indent", dump_options.json_indent, "JSON indentation level");
  dump->add_option("files", dump_files, "Files to read into the stack");

  // Check Command:
  engine::CheckOptions check_options{};
  std::vector<std::string> check_files;
  auto* check = app.add_subcommand("check", "Validate stack file configurations.");
  check->add_flag("-s,--summarize", check_options.summarize, "Summarize results");
  check->add_flag("-j,--json", check_options.output_json, "Output results as JSON data");
  check->add_option("-J,--json-indent", check_options.json_indent, "JSON indentation level");
  check->add_option("files", check_files, "Files to check");

  engine::ProbeOptions probe_options{};
  std::vector<std::string> probe_files{};
  auto* probe = app.add_subcommand("probe", "Probe a simulation with (merged) stack files.");
  probe->add_option("-J,--json-indent", probe_options.json_indent, "JSON indentation level");
  probe->add_option("files", probe_files, "Files to merge into a single stackfile")->required();

  // Run Command:
  engine::RunOptions run_options{};
  std::vector<std::string> run_files{};
  auto* run = app.add_subcommand("run", "Run a simulation with (merged) stack files.");
  run->add_option("-J,--json-indent", run_options.json_indent, "JSON indentation level");
  run->add_option("-u,--uuid", run_options.uuid, "Override simulation UUID")
      ->envname("CLOE_SIMULATION_UUID");
  run->add_flag("--allow-empty", run_options.allow_empty, "Allow empty simulations");
  run->add_flag("-w,--write-output,!--no-write-output", run_options.write_output,
                "Do (not) write any output files")
      ->envname("CLOE_WRITE_OUTPUT");
  run->add_option("-o,--output-path", run_options.output_path, "Write output to given directory")
      ->envname("CLOE_OUTPUT_PATH");
  run->add_flag("--progress,!--no-progress", run_options.report_progress,
                "Do (not) report progress");
  run->add_flag("--require-success,!--no-require-success", run_options.require_success,
                "Require simulation success")
      ->envname("CLOE_REQUIRE_SUCCESS");
  run->add_flag("--debug-lua", run_options.debug_lua, "Debug the Lua simulation");
  run->add_option("--debug-lua-port", run_options.debug_lua_port,
                  "Port to listen on for debugger to attach to")
      ->envname("CLOE_DEBUG_LUA_PORT");
  run->add_option("files", run_files, "Files to merge into a single stackfile")->required();

  // One of the above subcommands must be used.
  app.require_subcommand();

  // Shell Command:
  engine::ShellOptions shell_options{};
  std::vector<std::string> shell_files{};
  auto* shell = app.add_subcommand("shell", "Start a Lua shell.");
  shell->add_flag("-i,--interactive,!--no-interactive", shell_options.interactive,
                  "Drop into interactive mode (default)");
  shell->add_option("-c,--command", shell_options.commands, "Lua to run after running files");
  shell->add_option("files", shell_files, "Lua files to run before starting the shell");

  // Global Options:
  std::string log_level = "warn";
  app.set_help_all_flag("-H,--help-all", "Print all help messages and exit");
  app.add_option("-l,--level", log_level,
                 "Default logging level, one of [trace, debug, info, warn, error, critical]")
      ->envname("CLOE_LOG_LEVEL");

  // Stack Options:
  cloe::StackOptions stack_options{};
  stack_options.environment = std::make_unique<fable::Environment>();
  app.add_option("-p,--plugin-path", stack_options.plugin_paths,
                 "Scan additional directory for plugins (Env:CLOE_PLUGIN_PATH)");
  app.add_option("-i,--ignore", stack_options.ignore_sections,
                 "Ignore sections by JSON pointer syntax");
  app.add_flag("--no-builtin-plugins", stack_options.no_builtin_plugins,
               "Disable built-in plugins");
  app.add_flag("--no-system-plugins", stack_options.no_system_plugins,
               "Disable automatic loading of system plugins");
  app.add_flag("--no-system-confs", stack_options.no_system_confs,
               "Disable automatic sourcing of system configurations");
  app.add_flag("--no-hooks", stack_options.no_hooks, "Disable execution of hooks");
  app.add_flag("!--no-interpolate", stack_options.interpolate_vars,
               "Interpolate variables of the form ${XYZ} in stack files");
  app.add_flag("--interpolate-undefined", stack_options.interpolate_undefined,
               "Interpolate undefined variables with empty strings");

  cloe::LuaOptions lua_options{};
  lua_options.environment = stack_options.environment;
  app.add_option("--lua-path", lua_options.lua_paths,
                 "Scan directory for lua files when loading modules (Env:CLOE_LUA_PATH)");
  app.add_flag("--no-system-lua", lua_options.no_system_lua, "Disable default Lua system paths");

  // The --strict flag here is useful for all our smoketests, since this is the
  // combination of flags we use for maximum reproducibility / isolation.
  // Note: This option also affects / overwrites options for the run subcommand!
  app.add_flag(
         "-t,--strict,!--no-strict", stack_options.strict_mode,
         "Forces flags: --no-system-plugins --no-system-confs --no-system-lua --require-success")
      ->envname("CLOE_STRICT_MODE");
  app.add_flag("-s,--secure,!--no-secure", stack_options.secure_mode,
               "Forces flags: --strict --no-hooks --no-interpolate")
      ->envname("CLOE_SECURE_MODE");

  // ----------------------------------------------------------------------- //

  CLI11_PARSE(app, argc, argv);

  // Set logging pattern and intensity.
  // The currently configured pattern will result in lines that look like:
  //
  //     II 14:11:31.089 [cloe/triggers] Register action: something/hmi
  //
  // If using color is undesirable, the pattern can be manually set to
  // not make use of color codes, by removing the initial %^ and ending %$.
  {
    spdlog::set_pattern("%^%L%L %H:%M:%S.%e [%n] %v%$");
    auto level = cloe::logger::into_level(log_level);
    cloe::logger::set_level(level);
  }

  // Setup stack, applying strict/secure mode if necessary, and provide launch command.
  {
    if (stack_options.secure_mode) {
      stack_options.strict_mode = true;
      stack_options.no_hooks = true;
      stack_options.interpolate_vars = false;
    }
    if (stack_options.strict_mode) {
      stack_options.no_system_plugins = true;
      stack_options.no_system_confs = true;
      lua_options.no_system_lua = true;
      run_options.require_success = true;
    }

    stack_options.environment->prefer_external(false);
    stack_options.environment->allow_undefined(stack_options.interpolate_undefined);
    stack_options.environment->insert(CLOE_SIMULATION_UUID_VAR, "${" CLOE_SIMULATION_UUID_VAR "}");
  }

  auto with_global_options = [&](auto& opt) -> decltype(opt) {
    std::swap(opt.stack_options, stack_options);
    std::swap(opt.lua_options, lua_options);
    return opt;
  };

  // Errors should be handled by the subcommands, but logic errors and system
  // exceptions should cascade to the top so we can handle them.
  try {
    if (*version) {
      return engine::version(version_options);
    } else if (*usage) {
      return engine::usage(with_global_options(usage_options), usage_key_or_path);
    } else if (*dump) {
      return engine::dump(with_global_options(dump_options), dump_files);
    } else if (*check) {
      return engine::check(with_global_options(check_options), check_files);
    } else if (*probe) {
      return engine::probe(with_global_options(probe_options), probe_files);
    } else if (*run) {
      return engine::run(with_global_options(run_options), run_files);
    } else if (*shell) {
      return engine::shell(with_global_options(shell_options), shell_files);
    }
  } catch (cloe::ConcludedError& e) {
    return EXIT_FAILURE;
  } catch (std::exception& e) {
    bool is_logic_error = false;
    if (dynamic_cast<std::logic_error*>(&e) != nullptr) {
      is_logic_error = true;
    }

    std::cerr << "\n"
              << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
              << std::flush;

    if (is_logic_error) {
      std::cerr << "Fatal logic error:\n"
                << "\n"
                << "    " << e.what() << "\n"
                << "\n"
                << "This should never occur and is most likely a bug.\n"
                << "Please report this error to: " << CLOE_CONTACT_EMAIL << "\n"
                << "\n"
                << std::flush;
    } else {
      std::cerr << "Fatal error:\n"
                << "\n"
                << "    " << e.what() << "\n"
                << "\n"
                << "Consider inspecting the core dump for more information.\n"
                << std::flush;
    }

    std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
              << std::endl;
  }

  return EXIT_FAILURE;
}
