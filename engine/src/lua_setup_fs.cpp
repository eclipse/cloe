/*
 * Copyright 2023 Robert Bosch GmbH
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

#include "lua_setup.hpp"

#include <filesystem>  // for path
namespace fs = std::filesystem;

#include <sol/table.hpp>

namespace cloe {

namespace {

std::string basename(const std::string& file) { return fs::path(file).filename().generic_string(); }

std::string dirname(const std::string& file) {
  return fs::path(file).parent_path().generic_string();
}

std::string normalize(const std::string& file) {
  return fs::weakly_canonical(fs::path(file)).generic_string();
}

std::string realpath(const std::string& file) {
  std::error_code ec;
  auto p = fs::canonical(fs::path(file), ec);
  if (ec) {
    // FIXME: Implement proper error handling for Lua API.
    return "";
  }
  return p.generic_string();
}

std::string join(const std::string& file_left, const std::string& file_right) {
  return (fs::path(file_left) / fs::path(file_right)).generic_string();
}

bool is_absolute(const std::string& file) { return fs::path(file).is_absolute(); }

bool is_relative(const std::string& file) { return fs::path(file).is_relative(); }

bool is_dir(const std::string& file) { return fs::is_directory(fs::path(file)); }

bool is_file(const std::string& file) { return fs::is_regular_file(fs::path(file)); }

bool is_symlink(const std::string& file) { return fs::is_symlink(fs::path(file)); }

// It is NOT a directory, regular file, or symlink.
// Therefore, it is either a
// - block file,
// - character file,
// - fifo pipe, or
// - socket.
bool is_other(const std::string& file) { return fs::is_other(fs::path(file)); }

bool exists(const std::string& file) { return fs::exists(fs::path(file)); }

}  // anonymous namespace

void register_lib_fs(sol::table& lua) {
  lua.set_function("basename", basename);
  lua.set_function("dirname", dirname);
  lua.set_function("normalize", normalize);
  lua.set_function("realpath", realpath);
  lua.set_function("join", join);

  lua.set_function("is_absolute", is_absolute);
  lua.set_function("is_relative", is_relative);
  lua.set_function("is_dir", is_dir);
  lua.set_function("is_file", is_file);
  lua.set_function("is_other", is_other);

  lua.set_function("exists", exists);
}

}  // namespace cloe
