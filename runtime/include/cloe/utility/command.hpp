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
 * \file cloe/utility/command.hpp
 * \see  cloe/utility/command.cpp
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector<>

#include <boost/filesystem/path.hpp>    // for path
#include <fable/schema.hpp>             // for Schema, Struct, Variant
#include <fable/schema/boost_path.hpp>  // for make_schema, Path

#include <cloe/core.hpp>  // for Confable, Schema

namespace cloe {

/**
 * Command describes the execution of a command.
 *
 * There are two ways a command can normally be executed:
 *
 * 1. Direct execution (executable + args).
 *    A system call is used to directly start the command as a child process
 *    passed the specific arguments defined.
 *
 * 2. Shell execution (command).
 *    An available shell is used to run the passed expression.
 *
 * Shell execution includes the interpretation of all sorts of symbols and may
 * include functions that are only available to the shell itself. For these
 * commands, no validation can occur pre-execution.
 */
class Command : public Confable {
 public:  // Types and Constructors
  /**
   * Execution mode.
   */
  enum class Mode {
    Sync,    /// run command and wait for completion
    Async,   /// run command in background but wait for completion at destruction
    Detach,  /// run command in background and detach from parent
  };

  /**
   * Logging verbosity.
   */
  enum class Verbosity {
    Never,    /// never log anything
    OnError,  /// log combined error when an error occurs
    Always,   /// log combined output
  };

  Command() = default;

  explicit Command(const std::string& command) {
    from_conf(Json{
        {"command", command},
    });
  }

  Command(boost::filesystem::path executable, std::initializer_list<std::string> args) {
    from_conf(Json{
        {"executable", executable.native()},
        {"args", args},
    });
  }

 public:  // Special
  /**
   * Return the executable.
   */
  boost::filesystem::path executable() const { return executable_; }

  /**
   * Return the executable arguments.
   */
  const std::vector<std::string>& args() const { return args_; }

  /**
   * Return the command as a string.
   *
   * This does not necessarily have the correct characters escaped so as to be
   * result in the same execution when pasted into a shell.
   */
  std::string command() const;

  Verbosity verbosity() const { return log_output_; }
  Command verbosity(Verbosity v) && {
    set_verbosity(v);
    return std::move(*this);
  }
  void set_verbosity(Verbosity v) { log_output_ = v; }

  Mode mode() const { return mode_; }
  Command mode(Mode m) && {
    set_mode(m);
    return std::move(*this);
  }
  void set_mode(Mode m) { mode_ = m; }

  Command sync() && { return std::move(*this).mode(Mode::Sync); }
  bool is_sync() const { return mode() == Mode::Sync; }

  Command async() && { return std::move(*this).mode(Mode::Async); }
  bool is_async() const { return mode() == Mode::Async; }

  Command detach() && { return std::move(*this).mode(Mode::Detach); }
  bool is_detach() const { return mode() == Mode::Detach; }

  bool ignore_failure() const { return ignore_failure_; }
  Command ignore_failure(bool v) && {
    set_ignore_failure(v);
    return std::move(*this);
  }
  void set_ignore_failure(bool v) { ignore_failure_ = v; }

 public:  // Confable Overrides
  CONFABLE_SCHEMA(Command) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    return Variant{
      Struct{
        {"path", make_schema(&executable_, "path to executable").require().not_empty().executable()},
        {"args", make_schema(&args_, "arguments to executable")},
        {"mode", make_schema(&mode_, "synchronization mode to use")},
        {"log_output", make_schema(&log_output_, "how to log command output")},
        {"ignore_failure", make_schema(&ignore_failure_, "whether to ignore execution failure")},
      },
      Struct{
        {"command", make_schema(&command_, "command to execute within shell").require().not_empty()},
        {"mode", make_schema(&mode_, "synchronization mode to use")},
        {"log_output", make_schema(&log_output_, "how to log command output")},
        {"ignore_failure", make_schema(&ignore_failure_, "whether to ignore execution failure")},
      },
      String{&command_, "command to execute within shell"}.not_empty(),
    };
    // clang-format on
  }

  void from_conf(const Conf& c) override;

 private:
  boost::filesystem::path executable_;
  std::vector<std::string> args_;
  std::string command_;
  Mode mode_ = Mode::Sync;
  Verbosity log_output_ = Verbosity::Always;
  bool ignore_failure_ = false;
};

// clang-format off
ENUM_SERIALIZATION(Command::Mode, ({
  {Command::Mode::Sync, "sync"},
  {Command::Mode::Async, "async"},
  {Command::Mode::Detach, "detach"},
}))

ENUM_SERIALIZATION(Command::Verbosity, ({
  {Command::Verbosity::Never, "never"},
  {Command::Verbosity::OnError, "on_error"},
  {Command::Verbosity::Always, "always"},
}))
// clang-format on

}  // namespace cloe
