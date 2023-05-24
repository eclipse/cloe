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

#include <fable/utility/path.hpp>

#include <filesystem>
#include <optional>

#if __unix__
#include <unistd.h>
#endif

#include <nlohmann/json.hpp>

#include <fable/utility/string.hpp>

namespace fable {

bool is_executable(const std::filesystem::path& path) {
  std::error_code ec;
  bool is_file = std::filesystem::is_regular_file(path, ec);
  if (ec) {
    return false;
  }

#if __unix__
  bool is_exe = ::access(path.c_str(), X_OK) == 0;
#else
  // On Windows things are executable by their file extension,
  // such as `.exe`, `.bat`, or `.cmd`, and this can be configured
  // in the registry, so there's not a simple way to check this here.
  // We'll just assume true.
  bool is_exe = true;
#endif
  return is_file && is_exe;
}

std::optional<std::filesystem::path> search_path(const std::filesystem::path& executable) {
  auto paths = split_string(std::getenv("PATH"), ":");
  for (const auto& dir : paths) {
    std::filesystem::path executable_path = std::filesystem::path(dir) / executable;
    if (is_executable(executable_path)) {
      return executable_path;
    }
  }
  return std::nullopt;
}

}  // namespace fable

namespace nlohmann {

void adl_serializer<std::filesystem::path>::to_json(json& j, const std::filesystem::path& p) {
  j = p.native();
}

void adl_serializer<std::filesystem::path>::from_json(const json& j, std::filesystem::path& p) {
  p = j.get<std::string>();
}

}  // namespace nlohmann
