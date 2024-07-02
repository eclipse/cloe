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
 * \file cloe/utility/xdg.cpp
 * \see  cloe/utility/xdg.hpp
 */

#include <cloe/utility/xdg.hpp>

#include <filesystem>  // for path, temp_directory_path
#include <sstream>     // for stringstream
#include <string>      // for string
#include <vector>      // for vector<>

#ifdef __linux__
#include <unistd.h>  // for getuid
#endif

namespace cloe::utility {

using std::filesystem::path;  // NOLINT

namespace {

path expand(const path& file) {
  if (file.empty()) {
    return file;
  }

  std::string s = file.string();
  if (s[0] == '~') {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
      throw XdgError::HOME_UNSET;
    }
    return path(home + s.substr(1, s.size() - 1));
  } else {
    return file;
  }
}

std::vector<path> split(const std::string& paths, char delim = '/') {
  std::stringstream ss;
  ss.str(paths);
  std::string item;
  std::vector<path> out;
  while (std::getline(ss, item, delim)) {
    out.push_back(expand(path(item)));
  }
  return out;
}

}  // anonymous namespace

#ifdef __linux__
path xdg_temp_dir() {
  path tmpdir = std::filesystem::temp_directory_path();
  return tmpdir / path("xdg-" + std::to_string(getuid()));
}
#endif

path xdg_getenv_path(const std::string& env) {
  const char* env_path_p = std::getenv(env.c_str());
  if (env_path_p != nullptr) {
    path env_path{env_path_p};
    if (env_path.is_absolute()) {
      return env_path;
    }
#ifdef THROW_RELATIVE_XDG_PATH_ERROR
    throw XdgError::RELATIVE_XDG_PATH;
#endif  // THROW_RELATIVE_XDG_PATH_ERROR
  }

  // Return an empty path.
  return path{};
}

path xdg_path(const std::string& env, const path& default_path) {
  auto p = xdg_getenv_path(env);
  if (!p.empty()) {
    return p;
  }

  // Return default path, but first see if we need to expand it.
  p = expand(default_path);
  if (p.empty()) {
    throw XdgError::EMPTY_DEFAULT;
  }
  return p;
}

std::vector<path> xdg_paths(const std::string& env, const std::string& default_paths) {
  const char* env_paths_p = std::getenv(env.c_str());
  if (env_paths_p != nullptr) {
    std::vector<path> out;
    for (auto& p : split(std::string(env_paths_p))) {
      if (p.is_absolute()) {
        out.push_back(p);
      }
#ifdef THROW_RELATIVE_XDG_PATH_ERROR
      throw XdgError::RELATIVE_XDG_PATH;
#endif  // THROW_RELATIVE_XDG_PATH_ERROR
    }
  }

  // Return default paths
  return split(default_paths);
}

path xdg_find(const path& file, const std::vector<path>& dirs) {
  for (const auto& dir : dirs) {
    if (std::filesystem::exists(dir / file)) {
      return dir / file;
    }
  }
  return path{};
}

std::vector<path> xdg_findall(const path& file, const std::vector<path>& dirs) {
  std::vector<path> out;
  for (const auto& dir : dirs) {
    if (std::filesystem::exists(dir / file)) {
      out.push_back(dir / file);
    }
  }
  return out;
}

void xdg_merge(const path& file, const std::vector<path>& dirs, bool rev,
               const std::function<bool(const path&)>& mergefn) {
  auto files = xdg_findall(file, dirs);
  if (rev) {
    for (auto it = files.rbegin(); it != files.rend(); it++) {
      if (!mergefn(*it)) {
        break;
      }
    }
  } else {
    for (auto it = files.begin(); it != files.end(); it++) {
      if (!mergefn(*it)) {
        break;
      }
    }
  }
}

}  // namespace cloe::utility
