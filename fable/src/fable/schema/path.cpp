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
#include <fable/schema/path_impl.hpp>
#include <fable/utility/path.hpp>

namespace fable::schema {

namespace detail {

const char* path_state_cstr(PathState e) {
  switch (e) {
    case PathState::Absent:
      return "path should not exist";
    case PathState::Exists:
      return "path should exist";
    case PathState::Executable:
      return "path should be executable";
    case PathState::FileExists:
      return "path should exist and be a file";
    case PathState::DirExists:
      return "path should exist and be a directory";
    case PathState::NotFile:
      return "path should either not exist or be a directory";
    case PathState::NotDir:
      return "path should either not exist or be a file";
    case PathState::Any:
      return "any path";
    default:
      throw std::logic_error("unreachable Path::State enumeration");
  }
}

template <>
bool exists(const std::filesystem::path& path) {
  return std::filesystem::exists(path);
}

template <>
bool is_regular_file(const std::filesystem::path& path) {
  return std::filesystem::is_regular_file(path);
}

template <>
bool is_directory(const std::filesystem::path& path) {
  return std::filesystem::is_directory(path);
}

template <>
bool is_other(const std::filesystem::path& path) {
  return std::filesystem::is_other(path);
}

template <>
std::filesystem::path canonical(const std::filesystem::path& path) {
  return std::filesystem::canonical(path);
}

template <>
std::optional<std::filesystem::path> search_path(const std::filesystem::path& executable) {
  return ::fable::search_path(executable);
}

}  // namespace detail

template class Path<std::filesystem::path>;

}  // namespace fable::schema
