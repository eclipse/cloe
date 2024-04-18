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
 * \file fable/conf.cpp
 * \see  fable/conf.hpp
 */

#include <fable/conf.hpp>

#include <filesystem>  // for path
#include <fstream>     // for ifstream
#include <string>      // for string
#include <vector>      // for vector<>

#include <fmt/format.h>  // for fmt::format

#include <fable/error.hpp>  // for ConfError, WrongType, MissingProperty
#include <fable/json.hpp>   // for NLOHMANN_JSON_ALLOW_COMMENTS

namespace fable {

Conf::Conf(std::string file) : file_(std::move(file)) {
  std::ifstream ifs(file_);
  if (ifs.fail()) {
    throw Error("could not open file {}: {}", file_, strerror(errno));
  }
  try {
    data_ = parse_json(ifs);
  } catch (std::exception& e) {
    throw Error("unable to parse file {}: {}", file_, e.what());
  }
}

bool Conf::has(const JsonPointer& key) const {
  try {
    return data_.contains(key);
  } catch (Json::exception&) {
    // Exception is probably one of json::out_of_range or json::parse_error.
    return false;
  }
}

Conf Conf::at(const std::string& key) const {
  return Conf{
      data_.at(key),
      file_,
      root_ + "/" + key,
  };
}

Conf Conf::at(const JsonPointer& ptr) const {
  return Conf{
      data_.at(ptr),
      file_,
      root_ + ptr.to_string(),
  };
}

size_t Conf::erase(const std::string& key) { return data_.erase(key); }

// NOLINTNEXTLINE(misc-no-recursion)
size_t Conf::erase(const JsonPointer& ptr, bool preserve_empty) {
  std::size_t n = 0;
  try {
    Json& parent = data_.at(ptr.parent_pointer());
    n = parent.erase(ptr.back());
    if (!preserve_empty && parent.empty() && !ptr.empty()) {
      n += erase(ptr.parent_pointer());
    }
  } catch (Json::exception&) {
    // Exception is probably one of json::out_of_range or json::parse_error.
    // If the key doesn't exist, then there is nothing we need to delete.
    std::ignore;
  }
  return n;
}

std::vector<Conf> Conf::to_array() const {
  assert_has_pointer_type("", JsonType::array);
  std::vector<Conf> output;
  auto n = data_.size();
  output.reserve(n);
  size_t i = 0;
  for (const auto& j : data_) {
    output.emplace_back(Conf{
        j,
        file_,
        root_ + "/" + std::to_string(i),
    });
    ++i;
  }
  return output;
}

void Conf::assert_has(const std::string& key) const {
  if (!has(key)) {
    throw_missing(key);
  }
}

void Conf::assert_has(const JsonPointer& ptr) const {
  if (!has(ptr)) {
    throw_missing(ptr);
  }
}

void Conf::assert_has_not(const std::string& key, const std::string& msg) const {
  if (has(key)) {
    if (msg.empty()) {
      throw_unexpected(key);
    } else {
      throw_error(msg);
    }
  }
}

void Conf::assert_has_not(const JsonPointer& ptr, const std::string& msg) const {
  if (has(ptr)) {
    throw_unexpected(ptr, msg);
  }
}

void Conf::assert_has_type(const std::string& key, JsonType t) const {
  assert_has(key);
  if (data_.at(key).type() != t) {
    throw_wrong_type(key, t);
  }
}

void Conf::assert_has_type(const JsonPointer& ptr, JsonType t) const {
  assert_has(ptr);
  if (data_.at(ptr).type() != t) {
    throw_wrong_type(ptr, t);
  }
}

std::filesystem::path Conf::resolve_file(const std::filesystem::path& filepath) const {
  namespace fs = std::filesystem;

  auto fp = filepath;
  if (fp.is_relative()) {
    if (is_from_file()) {
      assert(fs::exists(file_));
      fp = fs::path(file_).parent_path() / fp;
    } else {
      fp = fs::current_path() / fp;
    }
  }
  return fp;
}

std::string Conf::resolve_file(const std::string& filepath) const {
  return resolve_file(std::filesystem::path(filepath)).native();
}

[[noreturn]] void Conf::throw_error(const std::string& msg) const { throw ConfError{*this, msg}; }

[[noreturn]] void Conf::throw_unexpected(const std::string& key, const std::string& msg) const {
  if (msg.empty()) {
    throw error::UnexpectedProperty(*this, key);
  }
  throw ConfError{*this, msg};
}

[[noreturn]] void Conf::throw_unexpected(const JsonPointer& ptr, const std::string& msg) const {
  if (msg.empty()) {
    throw error::UnexpectedProperty(*this, ptr);
  }
  throw ConfError{*this, msg};
}

[[noreturn]] void Conf::throw_missing(const std::string& key) const {
  throw error::MissingProperty(*this, key);
}

[[noreturn]] void Conf::throw_missing(const JsonPointer& ptr) const {
  throw error::MissingProperty(*this, ptr);
}

[[noreturn]] void Conf::throw_wrong_type(const std::string& key) const {
  if (key.empty()) {
    throw error::WrongType(*this);
  }
  throw error::WrongType(*this, key);
}

[[noreturn]] void Conf::throw_wrong_type(const JsonPointer& ptr) const {
  if (ptr.empty()) {
    throw error::WrongType(*this);
  }
  throw error::WrongType(*this, ptr);
}

[[noreturn]] void Conf::throw_wrong_type(const std::string& key, JsonType expected) const {
  throw error::WrongType(*this, key, expected);
}

[[noreturn]] void Conf::throw_wrong_type(const JsonPointer& ptr, JsonType expected) const {
  throw error::WrongType(*this, ptr, expected);
}

}  // namespace fable
