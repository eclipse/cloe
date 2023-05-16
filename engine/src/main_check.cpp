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
#include <string>    // for string
#include <vector>    // for vector<>

#include <cloe/core/fable.hpp>

#include "main_commands.hpp"
#include "stack.hpp"

namespace engine {

/**
 * Output nothing in the case that a file is valid, and an error message if
 * there is a problem.
 *
 * This mirrors most closely the standard unix command-line philosophy.
 */
void check_stack(const cloe::StackOptions& opt, const std::vector<std::string>& files,
                        bool* ok = nullptr) {
  if (ok) {
    *ok = false;
  }
  cloe::Stack s = cloe::new_stack(opt, files);
  s.check_completeness();
  if (ok) {
    *ok = true;
  }
}

/**
 * Output a summary of its state, ranging from OK to FATAL.
 *
 * This is useful for those who want a definitive answer for the input.
 */
std::string check_summary(const CheckOptions& opt, const std::vector<std::string>& files,
                                 bool* ok = nullptr) {
  cloe::StackOptions stack_opt = opt.stack_options;
  stack_opt.error = nullptr;

  try {
    check_stack(stack_opt, files, ok);
    return "OK";
  } catch (cloe::StackIncompleteError& e) {
    return "INCOMPLETE (" + std::string(e.what()) + ")";
  } catch (cloe::ConfError& e) {
    return "INVALID (" + std::string(e.what()) + ")";
  } catch (std::exception& e) {
    return "ERROR (" + std::string(e.what()) + ")";
  }
}

/**
 * Output a JSON value of its state, with null returned for ok, and an
 * error object for each error.
 */
cloe::Json check_json(const CheckOptions& opt, const std::vector<std::string>& files,
                             bool* ok = nullptr) {
  cloe::StackOptions stack_opt = opt.stack_options;
  stack_opt.error = nullptr;

  if (opt.summarize) {
    return check_summary(opt, files, ok);
  } else {
    try {
      check_stack(stack_opt, files, ok);
      return nullptr;
    } catch (cloe::SchemaError& e) {
      return e;
    } catch (cloe::ConfError& e) {
      return e;
    } catch (std::exception& e) {
      return cloe::Json{
          {"error", e.what()},
      };
    }
  }
}

int check_merged(const CheckOptions& opt, const std::vector<std::string>& filepaths) {
  bool ok = false;
  if (opt.output_json) {
    *opt.output << check_json(opt, filepaths, &ok).dump(opt.json_indent) << std::endl;
  } else if (opt.summarize) {
    *opt.output << check_summary(opt, filepaths, &ok) << std::endl;
  } else {
    try {
      check_stack(opt.stack_options, filepaths, &ok);
    } catch (cloe::ConcludedError&) {
    } catch (std::exception& e) {
      *opt.output << e.what() << std::endl;
    }
  }

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int check(const CheckOptions& opt, const std::vector<std::string>& filepaths) {
  return check_merged(opt, filepaths);
}

}  // namespace engine
