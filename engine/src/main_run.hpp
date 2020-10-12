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
 * \file main_run.hpp
 * \see  main.cpp
 *
 * This file contains the "run" options and command.
 */

#pragma once

#include <csignal>   // for signal
#include <cstdlib>   // for getenv
#include <iostream>  // for cerr

// NOTE: Unfortunately, <boost/uuid/uuid_generators.hpp> includes Boost headers
// that make use of deprecated headers. This is fixed in Boost 1.70.0, which we
// cannot use until we migrate oak::Server away from cppnetlib.
// See: https://github.com/boostorg/random/issues/49
#define BOOST_ALLOW_DEPRECATED_HEADERS

#include <boost/lexical_cast.hpp>          // for lexical_cast
#include <boost/uuid/uuid_generators.hpp>  // for random_generator
#include <boost/uuid/uuid_io.hpp>

#include <cloe/core.hpp>      // for logger::get
#include <fable/utility.hpp>  // for read_conf

#include "main_stack.hpp"  // for Stack, new_stack
#include "simulation.hpp"  // for Simulation, SimulationResult
#include "stack.hpp"       // for Stack

namespace engine {

void handle_signal(int);

struct RunOptions {
  cloe::StackOptions stack_options;
  std::ostream& output = std::cout;
  std::ostream& error = std::cerr;

  // Options
  std::string uuid;

  // Flags:
  int json_indent = 2;
  bool allow_empty = false;
  bool write_output = true;
  bool require_success = false;
  bool report_progress = true;
};

Simulation* GLOBAL_SIMULATION_INSTANCE{nullptr};

template <typename Func>
auto handle_cloe_error(std::ostream& out, Func f) -> decltype(f()) {
  try {
    return f();
  } catch (cloe::Error& e) {
    out << "Error: " << e.what() << std::endl;
    if (e.has_explanation()) {
      out << "    Note:\n" << fable::indent_string(e.explanation(), "    ") << std::endl;
    }
    throw cloe::ConcludedError(e);
  }
}

inline int run(const RunOptions& opt, const std::vector<std::string>& filepaths) {
  cloe::logger::get("cloe")->info("Cloe {}", CLOE_ENGINE_VERSION);
  cloe::StackOptions stack_opt = opt.stack_options;

  // Set the UUID of the simulation:
  std::string uuid;
  if (!opt.uuid.empty()) {
    uuid = opt.uuid;
  } else if (std::getenv(CLOE_SIMULATION_UUID_VAR) != nullptr) {
    uuid = std::getenv(CLOE_SIMULATION_UUID_VAR);
  } else {
    uuid = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
  }
  stack_opt.environment->set(CLOE_SIMULATION_UUID_VAR, uuid);

  // Load the stack file:
  cloe::Stack s;
  try {
    handle_cloe_error(*stack_opt.error, [&]() {
      s = cloe::new_stack(stack_opt, filepaths);
      if (!opt.allow_empty) {
        s.check_completeness();
      }
    });
  } catch (cloe::ConcludedError& e) {
    return EXIT_FAILURE;
  }

  // Create simulation:
  Simulation sim(s, uuid);
  GLOBAL_SIMULATION_INSTANCE = &sim;
  std::signal(SIGINT, handle_signal);

  // Set options:
  sim.set_report_progress(opt.report_progress);

  // Run simulation:
  auto result = handle_cloe_error(*stack_opt.error, [&]() { return sim.run(); });
  if (result.outcome == SimulationOutcome::NoStart) {
    // If we didn't get past the initialization phase, don't output any
    // statistics or write any files, just go home.
    return EXIT_FAILURE;
  }

  // Write results:
  if (opt.write_output) {
    sim.write_output(result);
  }
  opt.output << cloe::Json(result).dump(opt.json_indent) << std::endl;

  switch (result.outcome) {
    case SimulationOutcome::Success:
      return EXIT_OUTCOME_SUCCESS;
    case SimulationOutcome::Stopped:
      return (opt.require_success ? EXIT_OUTCOME_STOPPED : EXIT_OUTCOME_SUCCESS);
    case SimulationOutcome::Aborted:
      return EXIT_OUTCOME_ABORTED;
    case SimulationOutcome::NoStart:
      return EXIT_OUTCOME_NOSTART;
    case SimulationOutcome::Failure:
      return EXIT_OUTCOME_FAILURE;
    default:
      return EXIT_OUTCOME_UNKNOWN;
  }
}

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
inline void handle_signal(int sig) {
  static size_t interrupts = 0;
  switch (sig) {
    case SIGSEGV:
    case SIGABRT:
      abort();
      break;
    case SIGINT:
    default:
      std::cerr << std::endl;  // print newline so that ^C is on its own line
      if (++interrupts == 3) {
        std::signal(sig, SIG_DFL);  // third time goes to the default handler
      }
      if (GLOBAL_SIMULATION_INSTANCE) {
        GLOBAL_SIMULATION_INSTANCE->signal_abort();
      }
      break;
  }
}

}  // namespace engine
