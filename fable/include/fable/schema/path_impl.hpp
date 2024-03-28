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
/**
 * \file fable/schema/path_impl.hpp
 * \see  fable/schema/path.hpp
 */

#pragma once

#include <fable/schema/path.hpp>

#include <limits>    // for numeric_limits
#include <optional>  // for optional<>
#include <regex>     // for regex, regex_match
#include <string>    // for string
#include <vector>    // for vector<>

#include <fable/environment.hpp>     // for interpolate_vars
#include <fable/utility/string.hpp>  // for split_string

namespace fable::schema {

namespace detail {

// Defined in path.cpp
const char* path_state_cstr(PathState e);

// The templates below are defined separately for
// boost::filesystem::path AND std::filesystem::path
template <typename TPath>
bool exists(const TPath& path);

template <typename TPath>
bool is_regular_file(const TPath& path);

template <typename TPath>
bool is_directory(const TPath& path);

template <typename TPath>
bool is_other(const TPath& path);

template <typename TPath>
TPath canonical(const TPath& path);

template <typename TPath>
std::optional<TPath> search_path(const TPath& executable);

}  // namespace detail

template <typename T>
Json Path<T>::json_schema() const {
  Json j{
      {"type", "string"},
  };
  if (!pattern_.empty()) {
    j["pattern"] = pattern_;
  }
  if (min_length_ != 0) {
    j["minLength"] = min_length_;
  }
  if (max_length_ != std::numeric_limits<size_t>::max()) {
    j["maxLength"] = max_length_;
  }

  if (req_state_ != State::Any) {
    j["comment"] = detail::path_state_cstr(req_state_);
  }

  this->augment_schema(j);
  return j;
}

template <typename T>
bool Path<T>::validate(const Conf& c, std::optional<SchemaError>& err) const {
  if (!this->validate_type(c, err)) {
    return false;
  }

  auto src = c.get<std::string>();
  if (interpolate_) {
    // XXX: Fix throw here
    src = interpolate_vars(src, env_);
  }
  if (src.size() < min_length_) {
    return this->set_error(err, c, "expect minimum path length of {}, got {}", min_length_,
                           src.size());
  }
  if (src.size() > max_length_) {
    return this->set_error(err, c, "expect maximum path length of {}, got {}", max_length_,
                           src.size());
  }
  if (!pattern_.empty() && !std::regex_match(src, std::regex(pattern_))) {
    return this->set_error(err, c, "expect path to match regex '{}': {}", pattern_, src);
  }

  Type p{src};
  if (req_abs_ && !p.is_absolute()) {
    return this->set_error(err, c, "expect path to be absolute: {}", src);
  }
  if (resolve_) {
    try {
      p = resolve_path(c, p);
    } catch (SchemaError& e) {
      err.emplace(std::move(e));
      return false;
    }
  }

  switch (req_state_) {
    case State::Absent:
      if (detail::exists(p)) {
        return this->set_error(err, c, detail::path_state_cstr(req_state_));
      }
      break;
    case State::Exists:
      if (!detail::exists(p)) {
        return this->set_error(err, c, detail::path_state_cstr(req_state_));
      }
      break;
    case State::Executable:
      // fallthrough
    case State::FileExists:
      if (!detail::is_regular_file(p)) {
        return this->set_error(err, c, detail::path_state_cstr(req_state_));
      }
      break;
    case State::DirExists:
      if (!detail::is_directory(p)) {
        return this->set_error(err, c, detail::path_state_cstr(req_state_));
      }
      break;
    case State::NotFile:
      if (detail::is_regular_file(p) || detail::is_other(p)) {
        return this->set_error(err, c, detail::path_state_cstr(req_state_));
      }
      break;
    case State::NotDir:
      if (detail::is_directory(p)) {
        return this->set_error(err, c, detail::path_state_cstr(req_state_));
      }
      break;
    default:
      break;
  }

  return true;
}

template <typename T>
typename Path<T>::Type Path<T>::deserialize(const Conf& c) const {
  auto s = c.get<std::string>();
  if (interpolate_) {
    s = interpolate_vars(s, env_);
  }
  Type p{s};
  if (resolve_) {
    p = resolve_path(c, p);
  }
  if (normalize_) {
    p = detail::canonical(p);
  }
  return p;
}

template <typename T>
typename Path<T>::Type Path<T>::resolve_path(const Conf& c, const Path<T>::Type& filepath) const {
  std::string filepath_str = filepath.generic_string();

  // Only resolve executables if the path is not a basename, otherwise we
  // let the search_path do the resolving.
  if (req_state_ == State::Executable && filepath_str.find('/') == std::string::npos) {
    auto result = detail::search_path(filepath);
    if (!result) {
      throw this->error(c, "expect executable to exist: {}", filepath.native());
    }
    return *result;
  } else {
    return c.resolve_file(filepath_str);
  }
}

}  // namespace fable::schema
