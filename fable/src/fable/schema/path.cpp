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
 * \file fable/schema/path.cpp
 * \see  fable/schema/path.hpp
 */

#include <fable/schema/path.hpp>

#include <limits>  // for numeric_limits
#include <regex>   // for regex, regex_match
#include <string>  // for string

#include <boost/filesystem.hpp>           // for path
#include <boost/process/search_path.hpp>  // for search_path
namespace fs = boost::filesystem;

#include <fable/environment.hpp>  // for interpolate_vars

namespace fable {
namespace schema {

namespace {

const char* path_state_cstr(Path::State e) {
  switch (e) {
    case Path::State::Absent:
      return "path should not exist";
    case Path::State::Exists:
      return "path should exist";
    case Path::State::Executable:
      return "path should be executable";
    case Path::State::FileExists:
      return "path should exist and be a file";
    case Path::State::DirExists:
      return "path should exist and be a directory";
    case Path::State::NotFile:
      return "path should either not exist or be a directory";
    case Path::State::NotDir:
      return "path should either not exist or be a file";
    case Path::State::Any:
      return "any path";
    default:
      throw std::logic_error("unreachable Path::State enumeration");
  }
}

}  // namespace

Json Path::json_schema() const {
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
    j["comment"] = path_state_cstr(req_state_);
  }

  this->augment_schema(j);
  return j;
}

void Path::validate(const Conf& c) const {
  this->validate_type(c);

  auto src = c.get<std::string>();
  if (interpolate_) {
    src = interpolate_vars(src, env_);
  }
  if (src.size() < min_length_) {
    this->throw_error(c, "expect minimum path length of {}, got {}", min_length_, src.size());
  }
  if (src.size() > max_length_) {
    this->throw_error(c, "expect maximum path length of {}, got {}", max_length_, src.size());
  }
  if (!pattern_.empty() && !std::regex_match(src, std::regex(pattern_))) {
    this->throw_error(c, "expect path to match regex '{}': {}", pattern_, src);
  }

  fs::path p{src};
  if (req_abs_ && !p.is_absolute()) {
    this->throw_error(c, "expect path to be absolute: {}", src);
  }
  if (resolve_) {
    p = resolve_path(c, p);
  }

  switch (req_state_) {
    case State::Absent:
      if (fs::exists(p)) {
        this->throw_error(c, path_state_cstr(req_state_));
      }
      break;
    case State::Exists:
      if (!fs::exists(p)) {
        this->throw_error(c, path_state_cstr(req_state_));
      }
      break;
    case State::Executable:
      // fallthrough
    case State::FileExists:
      if (!fs::is_regular_file(p)) {
        this->throw_error(c, path_state_cstr(req_state_));
      }
      break;
    case State::DirExists:
      if (!fs::is_directory(p)) {
        this->throw_error(c, path_state_cstr(req_state_));
      }
      break;
    case State::NotFile:
      if (fs::is_regular_file(p) || fs::is_other(p)) {
        this->throw_error(c, path_state_cstr(req_state_));
      }
      break;
    case State::NotDir:
      if (fs::is_directory(p)) {
        this->throw_error(c, path_state_cstr(req_state_));
      }
      break;
    default:
      break;
  }
}

Path::Type Path::deserialize(const Conf& c) const {
  auto s = c.get<std::string>();
  if (interpolate_) {
    s = interpolate_vars(s, env_);
  }
  fs::path p{s};
  if (resolve_) {
    p = resolve_path(c, p);
  }
  if (normalize_) {
    p = fs::canonical(p);
  }
  return p;
}

Path::Type Path::resolve_path(const Conf& c, const Path::Type& filepath) const {
  if (req_state_ == State::Executable) {
    // Only resolve executables if the path is not a basename, otherwise we
    // let the search_path do the resolving.
    if (filepath.generic_string().find("/") != std::string::npos) {
      return c.resolve_file(filepath);
    } else {
      auto result = boost::process::search_path(filepath);
      if (result.empty()) {
        this->throw_error(c, "expect executable to exist: {}", filepath.native());
      }
      return result;
    }
  } else {
    return c.resolve_file(filepath);
  }
}

}  // namespace schema
}  // namespace fable
