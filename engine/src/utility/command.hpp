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
 * \file utility/command.hpp
 * \see  utility/command.cpp
 *
 * This file contains several types that make use of cloe::Command.
 */

#pragma once

#include <string>        // for string
#include <system_error>  // for system_error
#include <vector>        // for vector<>

#include <boost/optional.hpp>        // for optional<>
#include <boost/process/child.hpp>   // for child
#include <cloe/core.hpp>             // for Logger, Json, Conf, ...
#include <cloe/trigger.hpp>          // for Action, ActionFactory, ...
#include <cloe/utility/command.hpp>  // for Command

namespace engine {

struct CommandResult {
  std::string name;
  std::string command;
  boost::optional<boost::process::child> child;
  boost::optional<int> exit_code;
  boost::optional<std::system_error> error;
  std::vector<std::string> output;
};

class CommandExecuter {
 public:
  explicit CommandExecuter(cloe::Logger logger, bool enabled = true)
      : logger_(logger), enabled_(enabled) {}

  bool is_enabled() const { return enabled_; }
  void set_enabled(bool v) { enabled_ = v; }

  CommandResult run_and_release(const cloe::Command&) const;

  void run(const cloe::Command&);

  void run_all(const std::vector<cloe::Command>& cmds);

  void wait(CommandResult&) const;

  void wait_all();

  std::vector<CommandResult> release_all();

  cloe::Logger logger() const { return logger_; }

 private:
  std::vector<CommandResult> handles_;
  cloe::Logger logger_{nullptr};
  bool enabled_{false};
};

namespace actions {

class Command : public cloe::Action {
 public:
  Command(const std::string& name, const cloe::Command& cmd, CommandExecuter* exec)
      : Action(name), command_(cmd), executer_(exec) {
    assert(executer_ != nullptr);
  }

  cloe::ActionPtr clone() const override {
    return std::make_unique<Command>(name(), command_, executer_);
  }

  cloe::CallbackResult operator()(const cloe::Sync&, cloe::TriggerRegistrar&) override;

 protected:
  void to_json(cloe::Json& j) const override;

 private:
  cloe::Command command_;
  CommandExecuter* executer_{nullptr};
};

class CommandFactory : public cloe::ActionFactory {
 public:
  using ActionType = Command;
  explicit CommandFactory(CommandExecuter* exec)
      : cloe::ActionFactory("command", "run a system command"), executer_(exec) {
    assert(executer_ != nullptr);
  }
  cloe::TriggerSchema schema() const override;
  cloe::ActionPtr make(const cloe::Conf& c) const override;
  cloe::ActionPtr make(const std::string& s) const override;

 private:
  CommandExecuter* executer_{nullptr};
};

}  // namespace actions
}  // namespace engine
