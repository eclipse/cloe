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
 * \file oak/route_muxer.hpp
 * \see  oak/route_muxer_test.cpp
 */

#pragma once

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace oak {

using Parameters = std::map<std::string, std::string>;

/**
 * The Muxer is an advanced map that takes HTTP endpoints as input and returns
 * a unique value.
 *
 * The kind of value that is returned is configurable, since Muxer doesn't
 * need to know. This allows for easy testing as well as the ability to return
 * complex web handlers.
 *
 * # Usage
 *
 * ```cpp
 * Muxer<bool> mux;
 * mux.set_default(false);
 * mux.set_backtrack(true);
 * mux.add("/index.html", true);
 * ```
 *
 * # Safety
 * As the muxer is almost always used in multi-threaded contexts, it contains
 * a read-write mutex that allows routes to be added dynamically.
 */
template <typename T>
class Muxer {
 public:
  /**
   * Converts a path spec to the normal form.
   *
   * In particular:
   * - trailing slashes are removed
   * - redundant path elements are removed
   * - everything following the first '?' is removed
   *
   * # Safety
   *
   * - This function should not panic.
   */
  static std::string normalize(const std::string& route) {
    std::string s = route.substr(0, route.find("?"));
    std::filesystem::path p(s);
    if (!p.is_absolute()) {
      return "";
    }
    s = p.lexically_normal().string();
    s = s.substr(0, s.find_last_not_of("/. \n\t\r") + 1);
    if (s.empty()) {
      return "/";
    }
    return s;
  }

  /**
   * Return true if the input param string does not contain any forbidden
   * characters.
   *
   * Legal characters are alphanumeric characters and one of `-_.`.
   */
  static bool is_identifier(const std::string& s) {
    auto is_illegal = [](char c) -> bool {
      return !(isalnum(c) || c == '_' || c == '-' || c == '.');
    };
    return std::find_if(s.begin(), s.end(), is_illegal) == s.end();
  }

  /**
   * Convert a path spec to an existing registered path, or "".
   *
   * If backtrack is true, then the first matching parent of the normalized
   * route is returned.
   */
  std::string resolve(const std::string& route) const {
    auto key = normalize(route);
    std::shared_lock read_lock(access_);
    if (!backtrack_) {
      if (routes_.count(key)) {
        return key;
      }
      return "";
    } else {
      std::filesystem::path p(key);
      while (!routes_.count(p.string())) {
        if (p.string() == "/") {
          return "";
        }
        p = p.parent_path();
      }
      return p.string();
    }
  }

  /**
   * Set the backtracking behavior.
   */
  void set_backtrack(bool enabled) {
    std::unique_lock write_lock(access_);
    backtrack_ = enabled;
  }

  /**
   * Set the default value, if no path can be matched.
   */
  void set_default(T def) { routes_[""] = def; }

  std::vector<std::string> routes() const {
    std::vector<std::string> vs;
    std::shared_lock read_lock(access_);
    for (const auto& kv : routes_) {
      if (kv.first.empty()) {
        continue;
      }
      vs.push_back(kv.first);
    }
    return vs;
  }

  bool has(const std::string& route) const {
    auto key = normalize(route);
    std::shared_lock read_lock(access_);
    return routes_.count(key) != 0;
  }

  void add(const std::string& route, T val) {
    auto key = normalize(route);
    std::unique_lock write_lock(access_);
    if (routes_.count(key)) {
      throw std::runtime_error("route already exists");
    }
    routes_[key] = val;
  }

  void set(const std::string& route, T val) {
    auto key = normalize(route);
    std::unique_lock write_lock(access_);
    routes_[key] = val;
  }

  /**
   * Get the value associated with the normalized and resolved route.
   *
   * If no default is set, this method will throw std::out_of_range when
   * a route is not registered.
   */
  std::pair<T, Parameters> get(const std::string& route) const {
    auto key = resolve(route);
    Parameters p{};
    std::shared_lock read_lock(access_);
    return std::make_pair(routes_.at(key), p);
  }

  void set_unsafe(const std::string& key, T val) {
    std::unique_lock write_lock(access_);
    routes_[key] = val;
  }

  std::pair<T, Parameters> get_unsafe(const std::string& key) const {
    Parameters p{};
    std::shared_lock read_lock(access_);
    return std::make_pair(routes_.at(key), p);
  }

 private:
  // Configuration:
  bool backtrack_ = false;

  // State:
  std::map<std::string, T> routes_;
  mutable std::shared_mutex access_;
};

}  // namespace oak
