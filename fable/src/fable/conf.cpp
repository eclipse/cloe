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

#include <fstream>  // for ifstream
#include <string>   // for string
#include <vector>   // for vector<>

#include <fmt/format.h>          // for fmt::format
#include <boost/filesystem.hpp>  // for path

#include <fable/error.hpp>  // for ConfError, WrongType, MissingProperty
#include <fable/json.hpp>   // for NLOHMANN_JSON_ALLOW_COMMENTS

namespace fable {

Conf::Conf(const std::string& file) : file_(file) {
  std::ifstream ifs(file_);
  if (ifs.fail()) {
    throw Error("could not open file {}: {}", file, strerror(errno));
  }
  try {
    data_ = parse_json(ifs);
  } catch (std::exception& e) {
    throw Error("unable to parse file {}: {}", file, e.what());
  }
}

bool Conf::has(const JsonPointer& key) const {
  try {
    return data_.contains(key);
  } catch (nlohmann::detail::exception&) {
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

Conf Conf::at(const JsonPointer& key) const {
  return Conf{
      data_.at(key),
      file_,
      root_ + key.to_string(),
  };
}

size_t Conf::erase(const std::string& key) { return data_.erase(key); }

size_t Conf::erase(const JsonPointer& key) {
  auto n = 0;
  try {
    Json& parent = data_.at(key.parent_pointer());
    // The const_cast is necessary because of a bug in the nlohmann::json_pointer type.
    n = parent.erase(const_cast<JsonPointer&>(key).back());
    if (parent.empty()) {
      n += erase(key.parent_pointer());
    }
  } catch (nlohmann::detail::exception&) {
    // Exception is probably one of json::out_of_range or json::parse_error.
    // If the key doesn't exist, then there is nothing we need to delete.
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

void Conf::assert_has(const JsonPointer& key) const {
  if (!has(key)) {
    throw_missing(key);
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

void Conf::assert_has_not(const JsonPointer& key, const std::string& msg) const {
  if (has(key)) {
    throw_unexpected(key, msg);
  }
}

void Conf::assert_has_type(const std::string& key, JsonType t) const {
  assert_has(key);
  if (data_.at(key).type() != t) {
    throw_wrong_type(key, t);
  }
}

void Conf::assert_has_type(const JsonPointer& key, JsonType t) const {
  assert_has(key);
  if (data_.at(key).type() != t) {
    throw_wrong_type(key, t);
  }
}

boost::filesystem::path Conf::resolve_file(const boost::filesystem::path& filepath) const {
  namespace fs = boost::filesystem;

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
  return resolve_file(boost::filesystem::path(filepath)).native();
}

[[noreturn]] void Conf::throw_error(const std::string& msg) const { throw ConfError{*this, msg}; }

[[noreturn]] void Conf::throw_unexpected(const std::string& key, const std::string& msg) const {
  if (msg.empty()) {
    throw error::UnexpectedProperty(*this, key);
  } else {
    throw ConfError{*this, msg};
  }
}

[[noreturn]] void Conf::throw_missing(const std::string& key) const {
  throw error::MissingProperty(*this, key);
}

[[noreturn]] void Conf::throw_wrong_type(const std::string& key) const {
  if (key.empty()) {
    throw error::WrongType(*this);
  } else {
    throw error::WrongType(*this, key);
  }
}

[[noreturn]] void Conf::throw_wrong_type(const std::string& key, JsonType type) const {
  throw error::WrongType(*this, key, type);
}

}  // namespace fable
