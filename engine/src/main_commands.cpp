/*
 * Copyright 2024 Robert Bosch GmbH
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
 * \file main_commands.cpp
 */

#include "main_commands.hpp"

#include <csignal>   // for signal
#include <cstdlib>   // for getenv
#include <iostream>  // for cerr
#include <tuple>     // for tuple

// NOTE: Unfortunately, <boost/uuid/uuid_generators.hpp> includes Boost headers
// that make use of deprecated headers. This is fixed in Boost 1.70.0, but
// we still need to support earlier versions of Boost.
#define BOOST_ALLOW_DEPRECATED_HEADERS

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>          // for lexical_cast
#include <boost/uuid/uuid_generators.hpp>  // for random_generator
#include <boost/uuid/uuid_io.hpp>

#include <cloe/core.hpp>      // for logger::get
#include <cloe/stack.hpp>     // for Stack
#include <fable/utility.hpp>  // for read_conf

#include "error_handler.hpp"  // for conclude_error
#include "simulation.hpp"     // for Simulation

namespace engine {

// We need a global instance so that our signal handler has access to it.
Simulation* GLOBAL_SIMULATION_INSTANCE{nullptr};  // NOLINT

void handle_signal(int sig) {
  static size_t interrupts = 0;
  switch (sig) {
    case SIGSEGV:
    case SIGABRT:
      abort();
      break;
    case SIGINT:
    default:
      std::cerr << "\n" << std::flush;  // print newline so that ^C is on its own line
      if (++interrupts == 3) {
        std::ignore = std::signal(sig, SIG_DFL);  // third time goes to the default handler
      }
      if (GLOBAL_SIMULATION_INSTANCE != nullptr) {
        GLOBAL_SIMULATION_INSTANCE->signal_abort();
      }
      break;
  }
}

// Set the UUID of the simulation:
template <typename Options>
std::string handle_uuid_impl(const Options& opt) {
  std::string uuid;
  if (!opt.uuid.empty()) {
    uuid = opt.uuid;
  } else if (std::getenv(CLOE_SIMULATION_UUID_VAR) != nullptr) {
    uuid = std::getenv(CLOE_SIMULATION_UUID_VAR);
  } else {
    uuid = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
  }
  opt.stack_options.environment->set(CLOE_SIMULATION_UUID_VAR, uuid);
  return uuid;
}

std::string handle_uuid(const RunOptions& opt) { return handle_uuid_impl(opt); }

std::string handle_uuid(const ProbeOptions& opt) { return handle_uuid_impl(opt); }

template <typename Options>
std::tuple<cloe::Stack, sol::state> handle_config_impl(const Options& opt,
                                                       const std::vector<std::string>& filepaths) {
  assert(opt.output != nullptr && opt.error != nullptr);
  auto log = cloe::logger::get("cloe");
  cloe::logger::get("cloe")->info("Cloe {}", CLOE_ENGINE_VERSION);

  // Load the stack file:
  sol::state lua_state;
  sol::state_view lua_view(lua_state.lua_state());
  cloe::Stack stack = cloe::new_stack(opt.stack_options);
  cloe::setup_lua(lua_view, opt.lua_options, stack);
#if CLOE_ENGINE_WITH_LRDB
  if (opt.debug_lua) {
    log->info("Lua debugger listening at port: {}", opt.debug_lua_port);
    cloe::start_lua_debugger(lua_view, opt.debug_lua_port);
  }
#else
  if (opt.debug_lua) {
    log->error("Lua debugger feature not available.");
  }
#endif
  cloe::conclude_error(*opt.stack_options.error, [&]() {
    for (const auto& file : filepaths) {
      if (boost::algorithm::ends_with(file, ".lua")) {
        cloe::merge_lua(lua_view, file);
      } else {
        cloe::merge_stack(opt.stack_options, stack, file);
      }
    }
  });

  return {std::move(stack), std::move(lua_state)};
}

std::tuple<cloe::Stack, sol::state> handle_config(const RunOptions& opt,
                                                  const std::vector<std::string>& filepaths) {
  return handle_config_impl(opt, filepaths);
}

std::tuple<cloe::Stack, sol::state> handle_config(const ProbeOptions& opt,
                                                  const std::vector<std::string>& filepaths) {
  return handle_config_impl(opt, filepaths);
}

}  // namespace engine
