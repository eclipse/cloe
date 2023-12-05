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
 * \file utility/command.cpp
 * \see  utility/command.hpp
 */

#include "utility/command.hpp"

#include <boost/process.hpp>  // for child, std_out, std_err
#include <stdexcept>          // for runtime_error

namespace engine {

CommandResult CommandExecuter::run_and_release(const cloe::Command& cmd) const {
  namespace bp = boost::process;

  // Verbosity is Never(0), OnFailure(1), Always(2)
  auto verbose = static_cast<int>(cmd.verbosity());

  CommandResult r;
  r.name = cmd.executable().filename().native();
  r.command = cmd.command();

  if (!enabled_) {
    logger()->warn("Running system commands disabled.");
    logger()->warn("> Command: {}", r.command);
    return r;
  }

  if (verbose > 1) {
    logger()->info("Run: {}", r.command);
  }
  try {
    if (!cmd.is_sync()) {
      r.child = bp::child(cmd.executable(), cmd.args());

      if (cmd.is_detach()) {
        r.child->detach();
        return r;
      }
    } else {
      bp::ipstream is;

      // The syntax `(bp::std_out & bp::std_err) > is` is valid and works, but
      // was only found by rummaging through the source code. Ridiculous.
      r.child = bp::child(cmd.executable(), cmd.args(), (bp::std_out & bp::std_err) > is);

      std::string line;
      // After finished running output the rest of the lines.
      while (std::getline(is, line)) {
        if (verbose > 1) {
          logger()->debug("{}:{} | {}", r.name, r.child->id(), line);
        }
        r.output.push_back(line);
      }

      r.child->wait();
      r.exit_code = r.child->exit_code();

      if (*r.exit_code != 0 && (verbose > 0 || !cmd.ignore_failure())) {
        logger()->error("Error running: {}", r.command);
        if (!r.output.empty()) {
          std::stringstream s;
          for (const auto& line : r.output) {
            s << "    " << line << "\n";
          }
          logger()->error("> Output:\n{}", s.str());
        }

        if (!cmd.ignore_failure()) {
          throw std::runtime_error("hook failed: " + r.command);
        }
      }
    }
  } catch (std::system_error& e) {
    if (verbose > 0 || !cmd.ignore_failure()) {
      logger()->error("Error running: {}", r.command);
      logger()->error("> Message: {}", e.what());

      if (!cmd.ignore_failure()) {
        throw cloe::ConcludedError{e};
      }
    }

    r.error = e;
  }

  return r;
}

void CommandExecuter::run(const cloe::Command& c) { handles_.emplace_back(run_and_release(c)); }

void CommandExecuter::run_all(const std::vector<cloe::Command>& cmds) {
  handles_.reserve(handles_.size() + cmds.size());
  for (const auto& c : cmds) {
    run(c);
  }
}

void CommandExecuter::wait(CommandResult& r) const {
  if (!r.exit_code && r.child) {
    logger()->info("Wait for {} [pid={}]", r.command, r.child->id());
    r.child->wait();
  }
}

void CommandExecuter::wait_all() {
  for (auto& r : handles_) {
    wait(r);
  }
}

namespace actions {

cloe::CallbackResult Command::operator()(const cloe::Sync&, cloe::TriggerRegistrar&) { executer_->run(command_); return cloe::CallbackResult::Ok; }

void Command::to_json(cloe::Json& j) const { command_.to_json(j); }

cloe::TriggerSchema CommandFactory::schema() const {
  static const char* desc = "system command to execute in default shell";
  return cloe::TriggerSchema{
      this->name(),
      this->description(),
      cloe::InlineSchema(desc, cloe::JsonType::string, true),
      cloe::make_prototype<cloe::Command>(),
  };
}

cloe::ActionPtr CommandFactory::make(const cloe::Conf& c) const {
  cloe::Conf conf{c};
  conf.erase("name");
  cloe::Command cmd;
  cmd.from_conf(conf);
  return std::make_unique<Command>(name(), cmd, executer_);
}

cloe::ActionPtr CommandFactory::make(const std::string& s) const {
  return make(cloe::Conf{cloe::Json{
      {"command", s},
  }});
}

}  // namespace actions
}  // namespace engine
