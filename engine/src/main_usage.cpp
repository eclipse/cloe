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

#include <iostream>  // for ostream
#include <memory>    // for shared_ptr<>
#include <string>    // for string
#include <utility>   // for pair<>, move
#include <vector>    // for vector<>

#include <cloe/utility/xdg.hpp>  // for find_all_config

#include "main_commands.hpp"  // for new_stack
#include "stack.hpp"          // for Stack

namespace engine {

void show_usage(const cloe::Stack& stack, std::ostream& out);
void show_plugin_usage(const cloe::Plugin& plugin, std::ostream& out, bool json, int indent);

int usage(const UsageOptions& opt, const std::string& argument) {
  cloe::Stack stack;
  try {
    stack = cloe::new_stack(opt.stack_options);
  } catch (cloe::ConcludedError& e) {
    return EXIT_FAILURE;
  }

  // Show requested usage.
  bool result = true;
  if (argument.empty()) {
    if (opt.output_json) {
      *opt.output << stack.schema().json_schema().dump(opt.json_indent) << std::endl;
    } else {
      show_usage(stack, *opt.output);
    }
  } else {
    std::shared_ptr<cloe::Plugin> plugin = stack.get_plugin_or_load(argument);
    show_plugin_usage(*plugin, *opt.output, opt.output_json, static_cast<int>(opt.json_indent));
  }
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

// --------------------------------------------------------------------------------------------- //

template <typename T>
void print_plugin_usage(std::ostream& out, const cloe::Plugin& plugin,
                        const std::string& prefix = "  ") {
  auto factory = plugin.make<T>();
  auto usage = factory->schema().usage_compact();
  out << dump_json(usage, prefix) << std::endl;
}

/**
 * Print a nicely formatted list of available plugins.
 *
 * Output looks like:
 *
 *     Available simulators:
 *       nop [builtin://simulator/nop]
 *
 *     Available controllers:
 *       basic [/path/to/basic.so]
 *       nop   [builtin://controller/nop]
 *
 *     Available components:
 *       noisy_lane_sensor [/path/to/noisy_lane_sensor.so]
 *       speedometer       [/path/to/speedometer.so]
 *
 */
void print_available_plugins(const cloe::Stack& stack, std::ostream& out,
                             const std::string& word = "Available") {
  const std::string prefix = "  ";
  auto print_available = [&](const std::string& type) {
    out << word << " " << type << "s:" << std::endl;

    std::vector<std::pair<std::string, std::string>> plugins;
    // Get and filter out plugins that are the wanted type.
    for (const auto& pair : stack.get_all_plugins()) {
      if (pair.second->type() == type) {
        plugins.emplace_back(pair.second->name(), pair.first);
      }
    }

    if (plugins.empty()) {
      out << prefix << "n/a" << std::endl << std::endl;
      return;
    }

    // Calculate how wide the first column needs to be:
    size_t max_length = 0;
    for (const auto& pair : plugins) {
      if (pair.first.size() > max_length) {
        max_length = pair.first.size();
      }
    }

    // Print the available names:
    for (const auto& pair : plugins) {
      auto name_len = pair.first.size();
      out << prefix << pair.first << std::string(max_length - name_len, ' ') << " [" << pair.second
          << "]\n";
    }
    out << std::endl;
  };

  print_available("simulator");
  print_available("controller");
  print_available("component");
}

/**
 * Print full program usage.
 */
void show_usage(const cloe::Stack& stack, std::ostream& out) {
  out << fmt::format("Cloe {} ({})", CLOE_ENGINE_VERSION, CLOE_ENGINE_TIMESTAMP) << std::endl;

  out << R"(
Cloe is a simulation middleware tool that ties multiple plugins together into a
cohesive and coherent simulation. This is performed based on JSON input that we
name "stack files".

In general, stack files are combined to form a single stack file configuration.
Thus it is possible to reduce duplicate information by including one stack file
from another, or augmenting a configuration on the command line by specifying
a further configuration.

By default, Cloe will include certain discovered system and user configuration
files by sourcing them, if they are available:

  /etc/xdg/cloe/config.json
  ${XDG_CONFIG_HOME-${HOME}/.config}/cloe/config.json

While this is useful for user- or system-specific configurations, it can be
undesirable when reproducibility is of importance, such as common during testing.
Thus, this behavior can be disabled with the --no-system-confs flag.

Several subcommands are available:

  version
    Show program version information.

    As a middleware solution, Cloe provides several interfaces that are versioned
    according to the semantic versioning standard (see https://semver.org/).
    The version command shows this information, along with other useful facts,
    such as the date of compilation. One of the most important version numbers is
    that of the stack file. This defines the format of the JSON schema, which all
    input stack files must match.

    Examples:
      cloe-engine version
      cloe-engine version -jJ4

  usage
    Show schema or plugin usage information.

    A stack file does not only contain configuration data for Cloe itself. Each
    component involved in the simulation is configured through the stack file.
    This command provides usage information for the entire stack file or for
    individual plugins. These plugins can be referred to by name, key, or path.
    When the --json flag is specified, the JSON schema is printed, which allows
    automatic validation of input stack files.

    Examples:
      cloe-engine usage -j
      cloe-engine usage builtin://controller/nop
      cloe-engine -p build/plugins usage basic
      cloe-engine usage -j build/plugins/controller_basic.so

  dump
    Dump configuration of merged stack files.

    A stack file as stored on disk does not necessarily represent the exact
    configuration that is used by Cloe, as default values are not specified and
    the stack file may include other stack files. This command prints the final
    merged configuration of a set of stack files. This is useful for guaranteeing
    future reproducibility or for debugging purposes.

    Examples:
      cloe-engine dump tests/config_nop_infinite.json

  check
    Validate individual or merged stack files.

    We may check any number of stack files to find errors before we run them.
    Plugins are loaded and used to validate the stack file to the fullest
    extent possible. Note that this cannot find errors that only exhibit at
    runtime, such as simulator that is inconsistently configured or a scenario
    that uses other vehicle names.

    The output from the check command follows the UNIX philosophy by default,
    but this can be altered with the --summarize option flag.

    Examples:
      cloe-engine check tests/test_nop_smoketest.json tests/option_timestep_60.json
      cloe-engine --no-system-confs check -ds tests/*.json

  run
    Run a single simulation with merged stack files.

    Examples:
      cloe-engine -l trace run cloe-stackfile.json debug-conf.json
      cloe-engine --no-system-confs -l warn run tests/build_config.json

Please report any bugs to: cloe-dev@eclipse.org

---

)";

  {
    auto files = cloe::utility::find_all_config(CLOE_XDG_SUFFIX "/config.json");
    if (files.empty()) {
      out << "Discovered default configuration files:" << std::endl;
      for (auto& file : files) {
        out << "  " << file.native() << std::endl;
      }
      out << std::endl;
    }
  }

  print_available_plugins(stack, out);
}

void show_plugin_usage(const cloe::Plugin& plugin, std::ostream& out, bool as_json, int indent) {
  auto factory = plugin.make<cloe::ModelFactory>();

  if (as_json) {
    cloe::Json json = factory->schema().json_schema_qualified(plugin.path());
    json["title"] = factory->name();
    json["description"] = factory->description();
    out << json.dump(indent) << std::endl;
    return;
  }

  out << "Name: " << factory->name() << std::endl;
  out << "Type: " << plugin.type() << std::endl;
  out << "Path: ";
  if (plugin.path().empty()) {
    out << "n/a" << std::endl;
  } else {
    out << plugin.path() << std::endl;
  }
  out << "Usage: " << factory->schema().usage().dump(indent) << std::endl;
  out << "Defaults: " << factory->to_json().dump(indent) << std::endl;
}

}  // namespace engine
