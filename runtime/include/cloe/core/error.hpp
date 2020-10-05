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
 * \file cloe/core/error.hpp
 * \see  cloe/core/error.cpp
 * \see  cloe/core.hpp
 */

#pragma once
#ifndef CLOE_CORE_ERROR_HPP_
#define CLOE_CORE_ERROR_HPP_

#include <stdexcept>  // for exception
#include <string>     // for string

#include <cloe/core/logger.hpp>  // for fmt::format

namespace cloe {

class Error : public std::exception {
 public:
  explicit Error(const std::string& what) : err_(what) {}
  explicit Error(const char* what) : err_(what) {}

  template <typename... Args>
  explicit Error(const char* format, const Args&... args) : err_(fmt::format(format, args...)) {}

  virtual ~Error() noexcept = default;

  const char* what() const noexcept override { return err_.what(); }

  bool has_explanation() const { return !explanation_.empty(); }

  void set_explanation(const std::string& explanation);

  template <typename... Args>
  void set_explanation(const char* format, const Args&... args) {
    set_explanation(fmt::format(format, args...));
  }

  const std::string& explanation() const { return explanation_; }

  Error explanation(const std::string& explanation) && {
    set_explanation(explanation);
    return std::move(*this);
  }

  template <typename... Args>
  Error explanation(const char* format, const Args&... args) && {
    set_explanation(fmt::format(format, args...));
    return std::move(*this);
  }

 private:
  std::runtime_error err_;
  std::string explanation_;
};

/**
 * ConcludedError can be used to signify that an error has already been logged
 * to the output.
 *
 * It is expected that the cause has already been logged at ERROR level.
 */
class ConcludedError : public std::exception {
 public:
  explicit ConcludedError(std::exception& e) : cause_(e) {}
  virtual ~ConcludedError() noexcept = default;

  std::exception& cause() const noexcept { return cause_; }
  const char* what() const noexcept override { return cause_.what(); }

 private:
  std::exception& cause_;
};

}  // namespace cloe

#endif  // CLOE_CORE_ERROR_HPP_
