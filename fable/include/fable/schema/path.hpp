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
 * \file fable/schema/path.hpp
 * \see  fable/schema/path.cpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <limits>   // for numeric_limits<>
#include <string>   // for string
#include <utility>  // for move

#include <boost/filesystem/path.hpp>  // for path

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {

// Forward declarations:
class Environment;  // from <fable/environment.hpp>

namespace schema {

/**
 * Path de-/serializes a string that represents a filesystem path.
 *
 * Filesystem paths are special strings. We must take things into consideration
 * such as:
 *
 * - how to interpret relative paths (as relative to current working directory
 *   or to the file containing the string)
 * - whether the path indicated exists or not
 * - whether the path refers to an executable
 * - whether to normalize the path (by making absolute)
 *
 * The Path schema type allows the user to specify these properties and will
 * validate these during deserialization.
 */
class Path : public Base<Path> {
 public:  // Types and Constructors
  using Type = boost::filesystem::path;

  /**
   * State represents valid states a path can be in relative to the filesystem.
   */
  enum class State {
    Any,         /// any valid path
    Absent,      /// path does not exist
    Exists,      /// path exists
    Executable,  /// path exists in search path or locally and has executable permission
    FileExists,  /// path exists and is a file
    DirExists,   /// path exists and is a directory
    NotFile,     /// path does not exist or is a directory
    NotDir,      /// path does not exist or is a file
  };

  Path(Type* ptr, std::string&& desc) : Base(JsonType::string, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  /**
   * Return the required state of the path in the filesystem.
   */
  State state() const { return req_state_; }

  /**
   * Set the required state of the path in the filesystem.
   */
  Path state(State s) && {
    req_state_ = s;
    return std::move(*this);
  }
  void set_state(State s) { req_state_ = s; }

  Path absent() && { return std::move(*this).state(State::Absent); }
  Path exists() && { return std::move(*this).state(State::Exists); }
  Path executable() && { return std::move(*this).state(State::Executable); }
  Path file_exists() && { return std::move(*this).state(State::FileExists); }
  Path dir_exists() && { return std::move(*this).state(State::DirExists); }
  Path not_file() && { return std::move(*this).state(State::NotFile); }
  Path not_dir() && { return std::move(*this).state(State::NotDir); }

  Path not_empty() && {
    set_min_length(1);
    return std::move(*this);
  }

  /**
   * Require the input path to be absolute.
   */
  Path absolute() && {
    req_abs_ = true;
    return std::move(*this);
  }

  /**
   * Return whether path resolution is active.
   *
   * By default this is true.
   */
  bool resolve() const { return resolve_; }

  /**
   * Set whether the configuration file location should be used to resolve the
   * path being set.
   *
   * This is particularly valuable when a configuration has references to other
   * files. Instead of using the current-working-directory as the reference
   * point, the file where the configuration value originates should be used
   * as the reference point.
   *
   * Resolve must be true if the search path should be used to resolve
   * executables.
   */
  Path resolve(bool value) && {
    resolve_ = value;
    return std::move(*this);
  }
  void set_resolve(bool value) { resolve_ = value; }

  bool normalize() const { return normalize_; }
  Path normalize(bool value) && {
    normalize_ = value;
    return std::move(*this);
  }
  void set_normalize(bool value) { normalize_ = value; }

  bool interpolate() const { return interpolate_; }
  void set_interpolate(bool value) { interpolate_ = value; }
  Path interpolate(bool value) && {
    interpolate_ = value;
    return std::move(*this);
  }

  Environment* environment() const { return env_; }
  void set_environment(Environment* env) { env_ = env; }
  Path environment(Environment* env) && {
    env_ = env;
    return std::move(*this);
  }

  size_t min_length() const { return min_length_; }
  void set_min_length(size_t value) { min_length_ = value; }
  Path min_length(size_t value) && {
    min_length_ = value;
    return std::move(*this);
  }

  size_t max_length() const { return max_length_; }
  void set_max_length(size_t value) { max_length_ = value; }
  Path max_length(size_t value) && {
    max_length_ = value;
    return std::move(*this);
  }

  const std::string& pattern() const { return pattern_; }
  void set_pattern(const std::string& value) { pattern_ = value; }
  Path pattern(const std::string& value) && {
    pattern_ = value;
    return std::move(*this);
  }

 public:  // Overrides
  Json json_schema() const override;
  void validate(const Conf& c) const override;

  using Interface::to_json;
  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = serialize(*ptr_);
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = deserialize(c);
  }

  Json serialize(const Type& x) const { return x.native(); }

  Type deserialize(const Conf& c) const;

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  Type resolve_path(const Conf&, const Type&) const;

 private:
  State req_state_{State::Any};
  bool req_abs_{false};
  bool resolve_{true};
  bool normalize_{false};
  bool interpolate_{false};
  size_t min_length_{0};
  size_t max_length_{std::numeric_limits<size_t>::max()};
  std::string pattern_{};
  Environment* env_{nullptr};
  Type* ptr_{nullptr};
};

inline Path make_schema_impl(boost::filesystem::path* ptr, std::string&& desc) {
  return Path(ptr, std::move(desc));
}

}  // namespace schema
}  // namespace fable

namespace nlohmann {

template <>
struct adl_serializer<boost::filesystem::path> {
  static void to_json(json& j, const boost::filesystem::path& p) { j = p.native(); }
  static void from_json(const json& j, boost::filesystem::path& p) { p = j.get<std::string>(); }
};

}  // namespace nlohmann
