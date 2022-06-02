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
 * \file main_stack.hpp
 * \see  main_stack.cpp
 */

#pragma once

#include <iostream>  // for ostream, cerr
#include <memory>    // for shared_ptr<>
#include <string>    // for string
#include <vector>    // for vector<>

#include <boost/optional.hpp>  // for optional<>

#include <fable/environment.hpp>  // for Environment
#include "stack.hpp"         // for Stack

namespace cloe {

// See main.cpp for descriptions of flags.
struct StackOptions {
  boost::optional<std::ostream&> error = std::cerr;
  std::shared_ptr<fable::Environment> environment;

  // Flags:
  std::vector<std::string> plugin_paths;
  std::vector<std::string> ignore_sections;
  bool no_builtin_plugins = false;
  bool no_system_plugins = false;
  bool no_system_confs = false;
  bool no_hooks = false;
  bool interpolate_vars = true;
  bool interpolate_undefined = false;
  bool strict_mode = false;
  bool secure_mode = false;
};

Stack new_stack(const StackOptions& opt);

Stack new_stack(const StackOptions& opt, const std::string& filepath);

Stack new_stack(const StackOptions& opt, const std::vector<std::string>& filepaths);

void merge_stack(const StackOptions& opt, Stack& s, const std::string& filepath);

}  // namespace cloe
