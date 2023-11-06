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
 * \file cloe/utility/xdg.hpp
 * \see  cloe/utility/xdg.cpp
 *
 * This file implements the basic XDG specification, as specified
 * [here](https://specifications.freedesktop.org/basedir-spec/basedir-spec-0.6.html).
 *
 * Inspiration for the API of this library taken from https://github.com/goulash/xdg.
 */

#pragma once

#include <filesystem>  // for path
#include <functional>  // for function
#include <string>      // for string
#include <vector>      // for vector<>

namespace cloe::utility {

/**
 * This enum class contains all the exceptions that can be thrown.
 */
enum class XdgError {
  /**
     * This error is thrown when we encounter a relative path where we expect
     * an absolute path, but only if the THROW_RELATIVE_XDG_PATH_ERROR is
     * defined.
     *
     * The XDG specification states:
     *
     *     All paths set in these environment variables must be absolute. If an
     *     implementation encounters a relative path in any of these variables
     *     it should consider the path invalid and ignore it.
     */
  RELATIVE_XDG_PATH,

  /**
     * When the HOME environment variable is unset, path expansion cannot
     * occur, and we throw this error.
     */
  HOME_UNSET,

  /**
     * When no default can be provided, then an environment variable MUST be
     * set, and we have no reasonable fallback.
     */
  EMPTY_DEFAULT
};

// The following xdg_* functions are implementation functions, and aren't meant
// to be used directly.
#ifdef __linux__
std::filesystem::path xdg_temp_dir();
#endif

std::filesystem::path xdg_getenv_path(const std::string& env);
std::filesystem::path xdg_path(const std::string& env, const std::filesystem::path& default_path);
std::vector<std::filesystem::path> xdg_paths(const std::string& env,
                                             const std::string& default_paths);

/*
 * Make sure to ask `is_empty()` on the result.
 */
std::filesystem::path xdg_find(const std::filesystem::path& file,
                               const std::vector<std::filesystem::path>& dirs);

std::vector<std::filesystem::path> xdg_findall(const std::filesystem::path& file,
                                               const std::vector<std::filesystem::path>& dirs);

void xdg_merge(const std::filesystem::path& file, const std::vector<std::filesystem::path>& dirs,
               bool reverse, const std::function<bool(const std::filesystem::path&)>& mergefn);

/// User configuration base directory, e.g., `~./config`.
inline std::filesystem::path config_home() {
#ifdef __linux__
  return xdg_path("XDG_CONFIG_HOME", "~/.config");
#elif WIN32
  return xdg_path("XDG_CONFIG_HOME", xdg_getenv_path("LOCALAPPDATA"));
#endif
}

/// User data files base directory, e.g., `~/.local/share`.
inline std::filesystem::path data_home() {
#ifdef __linux__
  return xdg_path("XDG_DATA_HOME", "~/.local/share");
#elif WIN32
  return xdg_path("XDG_DATA_HOME", xdg_getenv_path("LOCALAPPDATA"));
#endif
}

/// User cache files base directory, e.g., `~/.cache`.
inline std::filesystem::path cache_home() {
#ifdef __linux__
  return xdg_path("XDG_CACHE_HOME", "~/.cache");
#elif WIN32
  return xdg_path("XDG_CACHE_HOME", xdg_getenv_path("TEMP"));
#endif
}

/// User runtime files base directory, e.g., `/run/user/1000`
inline std::filesystem::path runtime_dir() {
#ifdef __linux__
  return xdg_path("XDG_RUNTIME_DIR", xdg_temp_dir());
#elif WIN32
  return xdg_path("XDG_CACHE_HOME", xdg_getenv_path("TEMP"));
#endif
}

/// Global configuration directories, e.g., `/etc/xdg`.
inline std::vector<std::filesystem::path> config_dirs() {
#ifdef __linux__
  return xdg_paths("XDG_CONFIG_DIRS", "/etc/xdg");
#elif WIN32
  return xdg_paths("XDG_CONFIG_DIRS", xdg_getenv_path("APPDATA").string());
#endif
}

/// Global data files directories, e.g., `/usr/local/share`.
inline std::vector<std::filesystem::path> data_dirs() {
#ifdef __linux__
  return xdg_paths("XDG_DATA_DIRS", "/usr/local/share:/usr/share");
#elif WIN32
  return xdg_paths("XDG_DATA_DIRS", xdg_getenv_path("APPDATA").string());
#endif
}

/// User and global configuration directories
inline std::vector<std::filesystem::path> all_config_dirs() {
  auto xs = config_dirs();
  xs.insert(xs.begin(), config_home());
  return xs;
}

/// User and global data directories
inline std::vector<std::filesystem::path> all_data_dirs() {
  auto xs = data_dirs();
  xs.insert(xs.begin(), data_home());
  return xs;
}

/*
 * The user_* functions return the correct path for the given suffix name.
 *
 * This is useful if you want to create a file.
 */
inline std::filesystem::path user_config(const std::filesystem::path& file) {
  return config_home() / file;
}
inline std::filesystem::path user_data(const std::filesystem::path& file) {
  return data_home() / file;
}
inline std::filesystem::path user_cache(const std::filesystem::path& file) {
  return cache_home() / file;
}
inline std::filesystem::path user_runtime(const std::filesystem::path& file) {
  return runtime_dir() / file;
}

/*
 * The find_* functions return the most relevant path for the given suffix
 * name that exists.
 *
 * This is useful if you want to read a file.
 */
inline std::filesystem::path find_config(const std::filesystem::path& file) {
  return xdg_find(file, all_config_dirs());
}
inline std::filesystem::path find_data(const std::filesystem::path& file) {
  return xdg_find(file, all_data_dirs());
}
inline std::filesystem::path find_cache(const std::filesystem::path& file) {
  return xdg_find(file, std::vector<std::filesystem::path>{cache_home()});
}
inline std::filesystem::path find_runtime(const std::filesystem::path& file) {
  return xdg_find(file, std::vector<std::filesystem::path>{runtime_dir()});
}

/*
 * The find_all_config and find_all_data functions return for config and data
 * all paths that contain the name suffix.
 *
 * This is useful if you can read multiple instances of the config or data.
 */
inline std::vector<std::filesystem::path> find_all_config(const std::filesystem::path& file) {
  return xdg_findall(file, all_config_dirs());
}
inline std::vector<std::filesystem::path> find_all_data(const std::filesystem::path& file) {
  return xdg_findall(file, all_data_dirs());
}

/*
 * The merge_config and merge_data follow the same logic as the find_all_*
 * functions, except that instead of returning the paths, they repeatedly
 * apply the supplied function to the path.
 *
 * Because in merging, the most important file should be loaded last, there
 * is a reverse option. If the function returns false, the merging is aborted.
 */
inline void merge_config(const std::filesystem::path& file,
                         const std::function<bool(const std::filesystem::path&)>& mergefn,
                         bool reverse = false) {
  xdg_merge(file, all_config_dirs(), reverse, mergefn);
}

inline void merge_data(const std::filesystem::path& file,
                       const std::function<bool(const std::filesystem::path&)>& mergefn,
                       bool reverse = false) {
  xdg_merge(file, all_data_dirs(), reverse, mergefn);
}

}  // namespace cloe::utility
