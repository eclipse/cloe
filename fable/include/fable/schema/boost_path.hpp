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
 * \file fable/schema/boost_path.hpp
 * \see  fable/schema/path.cpp
 */

#pragma once

#include <fable/schema/path.hpp>
#include <fable/schema/path_impl.hpp>
#include <fable/utility/boost_path.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/process/search_path.hpp>

namespace fable {
namespace schema {

template <>
struct is_path<boost::filesystem::path> : std::true_type {};

namespace detail {

template <>
inline bool exists(const boost::filesystem::path& path) {
  return boost::filesystem::exists(path);
}

template <>
inline bool is_regular_file(const boost::filesystem::path& path) {
  return boost::filesystem::is_regular_file(path);
}

template <>
inline bool is_directory(const boost::filesystem::path& path) {
  return boost::filesystem::is_directory(path);
}

template <>
inline bool is_other(const boost::filesystem::path& path) {
  return boost::filesystem::is_other(path);
}

template <>
inline boost::filesystem::path canonical(const boost::filesystem::path& path) {
  return boost::filesystem::canonical(path);
}

template <>
inline std::optional<boost::filesystem::path> search_path(const boost::filesystem::path& executable) {
  boost::filesystem::path path = boost::process::search_path(executable);
  if (path.empty()) {
    return std::nullopt;
  }
  return path;
}

}  // namespace detail

}  // namespace schema
}  // namespace fable
