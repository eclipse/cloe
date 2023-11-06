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
 * \file cloe/utility/command.cpp
 * \see  cloe/utility/command.hpp
 */

#include <cloe/utility/command.hpp>

#include <cstdlib>     // for getenv
#include <filesystem>  // for path
#include <optional>    // for optional<>
#include <string>      // for string

#include <fable/utility/path.hpp>  // for search_path

namespace cloe {
namespace {

/**
 * Return the first suitable shell found, taking the SHELL environment variable
 * into account as a last resort.
 *
 * It might seem intuitive to take SHELL into account first, but this could
 * quickly lead to errors if the user starting Cloe makes use of an alternative
 * shell.
 */
std::filesystem::path shell_executable() {
  for (const auto* shell : {"sh", "bash", "dash", "zsh"}) {
    auto result = fable::search_path(shell);
    if (result) {
      return *result;
    }
  }

  if (std::getenv("SHELL") != nullptr) {
    auto result = fable::search_path(std::getenv("SHELL"));
    if (result) {
      return *result;
    }
  }

  return {""};
}

}  // namespace

std::string Command::command() const {
  std::string s = executable_.native();
  for (const auto& a : args_) {
    s += " " + a;
  }
  return s;
}

void Command::from_conf(const Conf& c) {
  command_.clear();
  Confable::from_conf(c);
  if (!command_.empty()) {
    executable_ = shell_executable();
    if (executable_.empty()) {
      c.throw_error("cannot find suitable shell to execute command");
    }
    args_.clear();
    args_.emplace_back("-c");
    args_.emplace_back(command_);
  }
}

}  // namespace cloe
