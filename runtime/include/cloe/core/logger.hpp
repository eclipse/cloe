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
 * \file cloe/core/logger.hpp
 * \see  cloe/core/logger.cpp
 * \see  cloe/core.hpp
 *
 * This file provides method for retrieving loggers for different namespaces
 * within the cloe library.
 *
 * The typical use-case of the functions in this file look like this:
 *
 * ```
 * auto log = logger::get("utility");
 * log->info("this is an informational message");
 * ```
 */

#pragma once
#ifndef CLOE_CORE_LOGGER_HPP_
#define CLOE_CORE_LOGGER_HPP_

#include <functional>  // for function<>
#include <memory>      // for shared_ptr<>
#include <string>      // for string

#include <fable/enum.hpp>  // for FABLE_ENUM_SERIALIZATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <spdlog/spdlog.h>
#pragma GCC diagnostic pop

// clang-format off
FABLE_ENUM_SERIALIZATION(spdlog::level::level_enum, ({
    {spdlog::level::level_enum::trace, "trace"},
    {spdlog::level::level_enum::debug, "debug"},
    {spdlog::level::level_enum::info, "info"},
    {spdlog::level::level_enum::warn, "warning"},
    {spdlog::level::level_enum::err, "error"},
    {spdlog::level::level_enum::critical, "fatal"},
    {spdlog::level::level_enum::off, "off"},
}))
// clang-format on

namespace cloe {

/**
 * Logger is a `shared_ptr` to an `spdlog::logger`.
 *
 * By default, this prints log messages to the console like so:
 *
 *     2017-04-20 08:37:03.787] [NAME] [LEVEL] MESSAGE
 *
 * where `NAME` is the name of the logger, `LEVEL` is the logging level,
 * and `MESSAGE` is the message to be printed.
 *
 * Levels available are:
 *
 *  - trace
 *  - debug
 *  - info
 *  - warn
 *  - error
 *  - critical
 *
 * Each of these levels can be used via a method of the same name.
 * For example:
 *
 * ```
 * auto log = logger::get("cloe")
 * log->set_level(spdlog::level::warning);  // only show warnings or higher
 * log->info("This is an informational message.")  // will not be shown
 * try {
 *   ...
 * } catch (exception& e) {
 *   log->error("Something bad happened: {}", e);
 * }
 * ```
 *
 * Formatting available for these functions is via the fmt library,
 * see http://fmtlib.net for an overview of the formatting supported.
 *
 * For a more detailed look at what the logger provides, see `<spdlog/logger.h>`.
 * Alternatively, the [Github](https://github.com/gabime/splog) site has more
 * documentation.
 */
using Logger = std::shared_ptr<spdlog::logger>;

/**
 * LogLevel represents the various severity levels of messages.
 * See Logger for more details on the levels.
 *
 * The individual levels can be accessed through spdlog::level.
 * For example,
 * ```
 * LogLevel l1 = spdlog::level::info;
 * auto l2 = spdlog::level::info;
 * assert(l1 == l2);
 * ```
 */
using LogLevel = spdlog::level::level_enum;

namespace logger {

/**
 * Return a logger for the given namespace name.
 *
 * If the logger does not exist, a new one will be created with the logger
 * factory. See the documentation on the Logger type for information on how to
 * use it.
 */
Logger get(std::string name);

/**
 * Sets the logger factory for loggers that do not exist yet.
 *
 * The default implementation uses an `spdlog::stdout_logger_mt`.
 * You can pass this function a lambda where you can use whatever logic
 * you like. If this function registers a logger with a name, then
 * this function will not be called again when getting that name, otherwise
 * it will. That makes voodoo like the following possible:
 *
 * ```
 * #include <cloe/core/logger.hpp>
 *
 * void main() {
 *   int count{}, created{};
 *   logger::set_factory([&](std::string name) {
 *     // Mangle all names so *I* have the control! Mwuahaha
 *     count++;
 *     name += "_mangled";
 *     auto l = spdlog::get(name);
 *     if (l) {
 *       // I already created it, so return this
 *       return l;
 *     }
 *
 *     // Logger does not exist yet, so create it how I want.
 *     auto log = spdlog::stdout_logger_mt(name);
 *     log->set_level(spdlog::level::error);  // only show errors or more severe
 *     log->set_pattern(">>> OWNED <<< [%H:%M:%S] %v");
 *     created++;
 *     return log;
 *   })
 *
 *   // ...
 *   // do stuff like get a logger and use it:
 *   auto log = logger::get("cloe");
 *   log->info("Super informational!");
 *   // ...
 *
 *   auto my_log = spdlog::stdout_logger_mt("special");  // bypass above
 *   my_log->info("Special logger factory called {} times and created {} loggers.", count, created);
 *   return 0;
 * }
 * ```
 *
 * You can pass in any function, function object, lambda, etc.
 */
void set_factory(std::function<Logger(std::string)> factory);

/**
 * Set the acceptable level of output for all loggers.
 *
 * This can be overriden on a per-logger basis. For example:
 * ```
 * logger:set_level(spdlog::level::error);
 * logger::get("cloe/webserver")->set_level(spdlog::level::info);
 * ```
 */
void set_level(LogLevel l);

/**
 * Convert the strings trace, debug, info, warn|warning, err|error,
 * critical|fatal, and off|disable into a logging level.
 */
LogLevel into_level(std::string s);

/**
 * Convert a logging level to one of the strings trace, debug, info, warn, err,
 * critical, and off.
 */
std::string to_string(LogLevel l);

}  // namespace logger
}  // namespace cloe

#endif  // CLOE_CORE_LOGGER_HPP_
