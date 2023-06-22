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
 * \file fable/error.hpp
 */

#pragma once

#include <stdexcept>  // for exception
#include <string>     // for string

#include <fmt/format.h>  // for format

#include <fable/conf.hpp>  // for Conf
#include <fable/fable_fwd.hpp>
#include <fable/json.hpp>  // for Json

namespace fable {

class Error;
class ConfError;
class SchemaError;

class Error : public std::exception {
 public:
  explicit Error(const std::string& what) : err_(what) {}
  explicit Error(const char* what) : err_(what) {}

  template <typename... Args>
  explicit Error(const char* format, const Args&... args) : err_(fmt::format(format, args...)) {}

  virtual ~Error() noexcept = default;

  const char* what() const noexcept override { return err_.what(); }

 private:
  std::runtime_error err_;
};

class ConfError : public Error {
  const Conf data_;

 public:
  virtual ~ConfError() noexcept = default;

  ConfError(const Conf& c, const std::string& msg) : Error(msg), data_(c) {}
  ConfError(const Conf& c, const char* msg) : Error(msg), data_(c) {}

  template <typename... Args>
  ConfError(const Conf& c, std::string_view format, Args&&... args)
      : Error(format, std::forward<Args>(args)...), data_(c) {}

  std::string file() const { return data_.file(); }
  std::string root() const { return data_.root(); }
  const Conf& conf() const { return data_; }
  const Json& data() const { return *data_; }

  virtual std::string message() const {
    return fmt::format("{}:{}: {}", file(), root(), this->what());
  }

  friend SchemaError;
  friend void to_json(Json& j, const ConfError& e) {
    j = Json{
        {"error", e.what()}, {"file", e.file()},       {"root", e.root()},
        {"data", e.data()},  {"message", e.message()},
    };
  }
};

namespace error {

inline ConfError MissingProperty(const Conf& c, const std::string& key) {
  return ConfError{c, "required property missing: {}", key};
}

inline ConfError UnexpectedProperty(const Conf& c, const std::string& key) {
  return ConfError{c, "unexpected property present: {}", key};
}

inline ConfError WrongType(const Conf& c, JsonType t) {
  std::string want = to_string(t);
  std::string got = to_string(c->type());
  return ConfError{c, "property must have type {}, got {}", want, got};
}

inline ConfError WrongType(const Conf& c, const std::string& key, JsonType t) {
  std::string want = to_string(t);
  std::string got = to_string((*c)[key].type());
  return ConfError{c, "property must have type {}, got {}", want, got};
}

inline ConfError WrongType(const Conf& c, const std::string& key) {
  std::string got = to_string((*c)[key].type());
  return ConfError{c, "property has wrong type {}", got};
}

inline ConfError WrongType(const Conf& c) {
  std::string got = to_string(c->type());
  return ConfError{c, "property has wrong type {}", got};
}

}  // namespace error

class SchemaError : public ConfError {
  const Json schema_;
  const Json context_;

 public:  // Constructors
  virtual ~SchemaError() noexcept = default;

  /**
   * Construct SchemaError with a ConfError.
   *
   * \param c ConfError
   * \param s Schema used for validation
   */
  SchemaError(const ConfError& c, const Json& s) : ConfError(c), schema_(s) {}

  /**
   * Construct SchemaError with a ConfError.
   *
   * \param c ConfError
   * \param s Schema used for validation
   * \param ctx Extra contextual data as JSON
   */
  SchemaError(const ConfError& c, const Json& s, const Json& ctx)
      : ConfError(c), schema_(s), context_(ctx) {}

  /**
   * Construct SchemaError.
   *
   * \param c Input Conf where error occurred
   * \param s Schema used for validation
   * \param format Message format string for fmt::format
   * \param args Arguments to message format
   */
  template <typename... Args>
  SchemaError(const Conf& c, const Json& s, std::string_view format, Args&&... args)
      : ConfError(c, format, std::forward<Args>(args)...), schema_(s) {}

  /**
   * Construct SchemaError.
   *
   * \param c Input Conf where error occurred
   * \param s Schema used for validation
   * \param ctx Extra contextual data as JSON
   * \param format Message format string for fmt::format
   * \param args Arguments to message format
   */
  template <typename... Args>
  SchemaError(const Conf& c, const Json& s, const Json& ctx, std::string_view format,
              Args&&... args)
      : ConfError(c, format, std::forward<Args>(args)...), schema_(s), context_(ctx) {}

 public:  // Special
  const Json& schema() const { return schema_; }
  const Json& context() const { return context_; }

  friend void to_json(Json& j, const SchemaError& e) {
    j = Json{
        {"error", e.what()}, {"file", e.file()},       {"root", e.root()},
        {"data", e.data()},  {"message", e.message()}, {"schema", e.schema_},
    };
    if (!e.context_.empty()) {
      j["context"] = e.context_;
    }
  }
};

}  // namespace fable
