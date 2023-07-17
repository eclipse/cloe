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
#include <utility>    // for move

#include <fmt/format.h>  // for format

#include <fable/conf.hpp>
#include <fable/fable_fwd.hpp>
#include <fable/json.hpp>

namespace fable {

class Error;
class ConfError;
class SchemaError;

class Error : public std::exception {
  std::string message_;

 public:
  Error() = delete;
  Error(const Error&) = default;
  Error(Error&&) = default;
  Error& operator=(const Error&) = default;
  Error& operator=(Error &&) = default;
  ~Error() noexcept override = default;

  Error(std::string what) : message_(std::move(what)) {}

  template <typename... Args>
  explicit Error(std::string_view format, Args&&... args)
      : message_(fmt::format(fmt::runtime(format), std::forward<Args>(args)...)) {}

  [[nodiscard]] const char* what() const noexcept override { return message_.c_str(); }
};

class ConfError : public Error {
  Conf data_;

 public:
  ConfError() = delete;
  ConfError(const ConfError&) = default;
  ConfError(ConfError&&) = default;
  ConfError& operator=(const ConfError&) = default;
  ConfError& operator=(ConfError &&) = default;
  ~ConfError() noexcept override = default;

  ConfError(Conf conf, const std::string& msg) : Error(msg), data_(std::move(conf)) {}
  ConfError(Conf conf, const char* msg) : Error(msg), data_(std::move(conf)) {}

  template <typename... Args>
  ConfError(Conf conf, std::string_view format, Args&&... args)
      : Error(format, std::forward<Args>(args)...), data_(std::move(conf)) {}

  [[nodiscard]] std::string file() const noexcept { return data_.file(); }
  [[nodiscard]] std::string root() const noexcept { return data_.root(); }
  [[nodiscard]] const Conf& conf() const noexcept { return data_; }
  [[nodiscard]] const Json& data() const noexcept { return *data_; }

  [[nodiscard]] virtual std::string message() const {
    return fmt::format("{}:{}: {}", file(), root(), this->what());
  }

  friend SchemaError;
  friend void to_json(Json& jref, const ConfError& err) {
    jref = Json{
        {"error", err.what()}, {"file", err.file()},       {"root", err.root()},
        {"data", err.data()},  {"message", err.message()},
    };
  }
};

namespace error {

inline ConfError MissingProperty(const Conf& conf, const std::string& key) {
  return ConfError{conf, "required property missing: {}", key};
}

inline ConfError UnexpectedProperty(const Conf& conf, const std::string& key) {
  return ConfError{conf, "unexpected property present: {}", key};
}

inline ConfError WrongType(const Conf& conf, JsonType type) {
  std::string want = to_string(type);
  std::string got = to_string(conf->type());
  return ConfError{conf, "property must have type {}, got {}", want, got};
}

inline ConfError WrongType(const Conf& conf, const std::string& key, JsonType type) {
  std::string want = to_string(type);
  std::string got = to_string((*conf)[key].type());
  return ConfError{conf, "property must have type {}, got {}", want, got};
}

inline ConfError WrongType(const Conf& conf, const std::string& key) {
  std::string got = to_string((*conf)[key].type());
  return ConfError{conf, "property has wrong type {}", got};
}

inline ConfError WrongType(const Conf& conf) {
  std::string got = to_string(conf->type());
  return ConfError{conf, "property has wrong type {}", got};
}

}  // namespace error

class SchemaError : public ConfError {
  Json schema_;
  Json context_;

 public:  // Constructors
  SchemaError() = delete;
  SchemaError(const SchemaError&) = default;
  SchemaError(SchemaError&&) = default;
  SchemaError& operator=(const SchemaError&) = default;
  SchemaError& operator=(SchemaError &&) = default;
  ~SchemaError() noexcept override = default;

  /**
   * Construct SchemaError with a ConfError.
   *
   * \param err ConfError
   * \param schema Schema used for validation
   */
  SchemaError(const ConfError& err, Json schema) : ConfError(err), schema_(std::move(schema)) {}

  /**
   * Construct SchemaError.
   *
   * \param conf Input Conf where error occurred
   * \param schema Schema used for validation
   * \param format Message format string for fmt::format
   * \param args Arguments to message format
   */
  template <typename... Args>
  SchemaError(const Conf& conf, Json schema, std::string_view format, Args&&... args)
      : ConfError(conf, format, std::forward<Args>(args)...), schema_(std::move(schema)) {}

  [[nodiscard]] const Json& schema() const { return schema_; }
  [[nodiscard]] const Json& context() const { return context_; }

  SchemaError& with_context(Json ctx) {
    context_ = std::move(ctx);
    return *this;
  }

  friend void to_json(Json& json, const SchemaError& err) {
    json = Json{
        {"error", err.what()}, {"file", err.file()},       {"root", err.root()},
        {"data", err.data()},  {"message", err.message()}, {"schema", err.schema_},
    };
    if (!err.context_.empty()) {
      json["context"] = err.context_;
    }
  }
};

}  // namespace fable
