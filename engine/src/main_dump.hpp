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
 * \file main_dump.hpp
 * \see  main.cpp
 *
 * This file contains the "dump" options and commands.
 */

#pragma once

#include <iostream>  // for ostream, cout
#include <string>    // for string
#include <vector>    // for vector<>

#include "main_stack.hpp"  // for Stack, new_stack

namespace engine {

struct DumpOptions {
  cloe::StackOptions stack_options;
  std::ostream& output = std::cout;

  // Flags:
  int json_indent = 2;
};

inline int dump(const DumpOptions& opt, const std::vector<std::string>& filepaths) {
  try {
    cloe::Stack s = cloe::new_stack(opt.stack_options, filepaths);
    opt.output << s.to_json().dump(opt.json_indent) << std::endl;
    return EXIT_SUCCESS;
  } catch (cloe::ConcludedError& e) {
    return EXIT_FAILURE;
  }
}

}  // namespace engine
