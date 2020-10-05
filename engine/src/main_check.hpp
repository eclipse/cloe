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
 * \file main_check.hpp
 * \see  main.cpp
 *
 * This file contains the "check" options and command.
 */

#pragma once

#include <iostream>  // for ostream, cout
#include <string>    // for string
#include <vector>    // for vector<>

#include "main_stack.hpp"  // for Stack, StackOptions, new_stack

namespace engine {

struct CheckOptions {
  cloe::StackOptions stack_options;
  std::ostream& output = std::cout;
  std::string delimiter = ",";

  // Flags:
  bool distinct = false;
  bool summarize = false;
  bool output_json = false;
  int json_indent = 2;
};

/**
 * Output nothing in the case that a file is valid, and an error message if
 * there is a problem.
 *
 * This mirrors most closely the standard unix command-line philosophy.
 */
inline void check_stack(const cloe::StackOptions& opt, const std::vector<std::string>& files,
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
inline std::string check_summary(const CheckOptions& opt, const std::vector<std::string>& files,
                                 bool* ok = nullptr) {
  cloe::StackOptions stack_opt = opt.stack_options;
  stack_opt.error = boost::none;

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
inline cloe::Json check_json(const CheckOptions& opt, const std::vector<std::string>& files,
                             bool* ok = nullptr) {
  cloe::StackOptions stack_opt = opt.stack_options;
  stack_opt.error = boost::none;

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

inline int check_merged(const CheckOptions& opt, const std::vector<std::string>& filepaths) {
  bool ok = false;
  if (opt.output_json) {
    opt.output << check_json(opt, filepaths, &ok).dump(opt.json_indent) << std::endl;
  } else if (opt.summarize) {
    opt.output << check_summary(opt, filepaths, &ok) << std::endl;
  } else {
    try {
      check_stack(opt.stack_options, filepaths, &ok);
    } catch (cloe::ConcludedError&) {
    } catch (std::exception& e) {
      opt.output << e.what() << std::endl;
    }
  }

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline int check_distinct(const CheckOptions& opt, const std::vector<std::string>& filepaths) {
  int exit_code = EXIT_SUCCESS;
  auto check_each = [&](std::function<void(const std::string&, bool*)> func) {
    for (const auto& x : filepaths) {
      bool ok = true;
      func(x, &ok);
      if (!ok) {
        exit_code = EXIT_FAILURE;
      }
    }
  };

  if (opt.output_json) {
    // Output for each file a summary
    cloe::Json output;
    check_each([&](const auto& f, bool* ok) {
      output[f] = check_json(opt, std::vector<std::string>{f}, ok);
    });
    opt.output << output.dump(opt.json_indent) << std::endl;
  } else if (opt.summarize) {
    check_each([&](const auto& f, bool* ok) {
      opt.output << f << ": " << check_summary(opt, std::vector<std::string>{f}, ok) << std::endl;
    });
  } else {
    check_each([&](const auto& f, bool* ok) {
      try {
        check_stack(opt.stack_options, std::vector<std::string>{f}, ok);
      } catch (cloe::ConcludedError&) {
      } catch (std::exception& e) {
        opt.output << f << ": " << e.what() << std::endl;
      }
    });
  }

  return exit_code;
}

inline int check(const CheckOptions& opt, const std::vector<std::string>& filepaths) {
  if (opt.distinct) {
    return check_distinct(opt, filepaths);
  } else {
    return check_merged(opt, filepaths);
  }
}

}  // namespace engine
