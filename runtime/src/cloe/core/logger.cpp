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
 * \file cloe/core/logger.cpp
 * \see  cloe/core/logger.hpp
 */

#include <cloe/core/logger.hpp>

#include <functional>  // for function
#include <memory>      // for make_shared
#include <string>      // for string

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <spdlog/common.h>                    // for get, set_level
#include <spdlog/sinks/ansicolor_sink.h>      // for ansicolor_stderr_sink_mt
#include <spdlog/sinks/stdout_color_sinks.h>  // for stderr_color_mt
#pragma GCC diagnostic pop

namespace cloe {
namespace logger {

namespace {

Logger defaultFactory(std::string name) {
  static const std::string gray = "\033[90m";
  auto logger = spdlog::stderr_color_mt(name);

  // Switch info level color from green to normal
  auto sink = dynamic_cast<spdlog::sinks::ansicolor_stderr_sink_mt*>(logger->sinks()[0].get());
  if (sink) {
    sink->set_color(spdlog::level::trace, gray);
    sink->set_color(spdlog::level::debug, gray);
    sink->set_color(spdlog::level::info, sink->reset);
  }

  return logger;
}

std::function<Logger(std::string)> logger_factory = &defaultFactory;

}  // anonymous namespace

Logger get(std::string name) {
  auto l = spdlog::get(name);
  if (l) {
    return l;
  }

  // l is empty, so we create a new logger based on default_logger
  return logger_factory(name);
}

void set_level(LogLevel l) { spdlog::set_level(l); }

void set_factory(std::function<Logger(std::string)> factory) { logger_factory = factory; }

LogLevel into_level(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  if (s == "trace") {
    return spdlog::level::trace;
  } else if (s == "debug") {
    return spdlog::level::debug;
  } else if (s == "info") {
    return spdlog::level::info;
  } else if (s == "warn" || s == "warning") {
    return spdlog::level::warn;
  } else if (s == "err" || s == "error") {
    return spdlog::level::err;
  } else if (s == "critical" || s == "fatal") {
    return spdlog::level::critical;
  } else if (s == "off" || s == "disable") {
    return spdlog::level::off;
  } else {
    throw std::runtime_error("can't convert string " + s + " to level");
  }
}

std::string to_string(LogLevel l) {
  switch (l) {
    case spdlog::level::trace:
      return "trace";
    case spdlog::level::debug:
      return "debug";
    case spdlog::level::info:
      return "info";
    case spdlog::level::warn:
      return "warn";
    case spdlog::level::err:
      return "err";
    case spdlog::level::critical:
      return "critical";
    case spdlog::level::off:
      return "off";
    default:
      throw std::logic_error("this logger level is unknown to me");
  }
}

}  // namespace logger
}  // namespace cloe
