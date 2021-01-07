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
 * \file main_stack.cpp
 * \see  main_stack.hpp
 */

#include "main_stack.hpp"

#include <iostream>  // for ostream, cerr
#include <string>    // for string
#include <vector>    // for vector<>

#include <boost/filesystem.hpp>  // for path
#include <boost/optional.hpp>    // for optional<>

#include <cloe/utility/std_extensions.hpp>  // for split_string
#include <cloe/utility/xdg.hpp>             // for merge_config
#include <fable/environment.hpp>            // for Environment
#include <fable/utility.hpp>                // for pretty_print, read_conf_from_file

#include "plugins/nop_controller.hpp"  // for NopFactory
#include "plugins/nop_simulator.hpp"   // for NopFactory
#include "stack.hpp"                   // for Stack

#define CLOE_PLUGIN_PATH "CLOE_PLUGIN_PATH"

namespace cloe {

Conf read_conf(const StackOptions& opt, const std::string& filepath) {
  if (!opt.interpolate_vars) {
    return fable::read_conf(filepath);
  }

  // Prepare environment with extra variables:
  fable::Environment env(*opt.environment);
  if (!filepath.empty() && filepath != "-") {
    std::string dirpath = boost::filesystem::canonical(filepath).parent_path().native();
    env.set("THIS_STACKFILE_FILE", filepath);
    env.set("THIS_STACKFILE_DIR", dirpath);
  }
  return fable::read_conf_with_interpolation(filepath, &env);
}

void merge_stack(const StackOptions& opt, Stack& s, const std::string& filepath) {
  auto merge = [&]() {
    Conf c = read_conf(opt, filepath);

    if (opt.no_hooks) {
      // Removing hooks from the config allows the stack to validate even if
      // the hooks themselves refer to commands that don't exist. This would
      // otherwise constitute an error.
      c.erase_pointer("/engine/hooks");
    }

    s.from_conf(c);
    s.validate();
  };

  if (!opt.error) {
    merge();
  } else {
    try {
      merge();
    } catch (SchemaError& e) {
      fable::pretty_print(e, *opt.error);
      throw ConcludedError{e};
    } catch (ConfError& e) {
      fable::pretty_print(e, *opt.error);
      throw ConcludedError{e};
    } catch (Error& e) {
      *opt.error << filepath << ": " << e.what() << std::endl;
      if (e.has_explanation()) {
        *opt.error << "    Note:\n" << fable::indent_string(e.explanation(), "    ") << std::endl;
      }
      throw ConcludedError{e};
    } catch (std::exception& e) {
      *opt.error << filepath << ": " << e.what() << std::endl;
      throw ConcludedError{e};
    }
  }
}

Stack new_stack(const StackOptions& opt) {
  Stack s;

  // Interpolate known variables, if requested.
  if (opt.interpolate_vars) {
    auto interpolate_path = [&opt](boost::optional<boost::filesystem::path>& p) {
      p = fable::interpolate_vars(p->native(), opt.environment.get());
    };
    interpolate_path(s.engine.registry_path);
    interpolate_path(s.engine.output_path);
    s.set_conf_reader(
        [&opt](const std::string& filepath) -> cloe::Conf { return read_conf(opt, filepath); });
  }

  // Insert ignored sections
  for (const auto& i : opt.ignore_sections) {
    s.engine.ignore_sections.emplace_back(i);
  }

  // Insert built-in plugins:
  if (!opt.no_builtin_plugins) {
    s.insert_plugin(make_plugin<controller::NopFactory>(), PluginConf{"builtin://controller/nop"});
    s.insert_plugin(make_plugin<simulator::NopFactory>(), PluginConf{"builtin://simulator/nop"});
  }

  // Setup plugin path:
  if (!opt.no_system_plugins) {
    s.engine.plugin_path = {
        "/usr/local/lib/cloe",
        "/usr/lib/cloe",
    };
  }
  std::string paths = opt.environment.get()->get_or(CLOE_PLUGIN_PATH, "");
  for (auto&& p : utility::split_string(std::move(paths), ":")) {
    s.engine.plugin_path.emplace_back(std::move(p));
  }
  for (const auto& p : opt.plugin_paths) {
    s.engine.plugin_path.emplace_back(p);
  }

  // Merge system configurations:
  if (!opt.no_system_confs) {
    auto mergefn = [&](const boost::filesystem::path& file) -> bool {
      s.logger()->info("Include conf {}", file.native());
      merge_stack(opt, s, file.native());
      return true;
    };
    cloe::utility::merge_config(CLOE_XDG_SUFFIX "/config.json", mergefn, true);
  }

  // Initialize configuration (scan and load plugins):
  s.initialize();

  return s;
}

Stack new_stack(const StackOptions& opt, const std::string& filepath) {
  Stack s = new_stack(opt);
  if (!filepath.empty()) {
    merge_stack(opt, s, filepath);
  }

  return s;
}

Stack new_stack(const StackOptions& opt, const std::vector<std::string>& filepaths) {
  Stack s = new_stack(opt);
  for (const auto& f : filepaths) {
    if (f.empty()) {
      continue;
    }
    merge_stack(opt, s, f);
  }

  return s;
}

}  // namespace cloe
