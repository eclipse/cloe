/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file server.hpp
 * \see  server.cpp
 */

#pragma once

#include <memory>  // for unique_ptr<>

#include <cloe/registrar.hpp>  // for Registrar
#include <cloe/stack.hpp>

#include "utility/defer.hpp"  // for Defer

namespace engine {

/**
 * Server registrar interface.
 *
 * This lets you register static and API endpoints with the server.
 * Get a new one from Server::server_registrar().
 */
class ServerRegistrar {
 public:
  virtual ~ServerRegistrar() = default;

  [[nodiscard]] virtual std::unique_ptr<ServerRegistrar> clone() const = 0;

  [[nodiscard]] virtual std::unique_ptr<ServerRegistrar> with_prefix(const std::string& static_prefix,
                                                       const std::string& api_prefix) const = 0;

  virtual void register_static_handler(const std::string& endpoint, cloe::Handler h) = 0;

  virtual void register_api_handler(const std::string& endpoint, cloe::HandlerType t,
                                    cloe::Handler h) = 0;
};

/**
 * Server interface to make altering the implementation easier.
 *
 * Use `make_server()` to create an instance that you can use.
 */
class Server {
 public:
  Server(const Server&) = default;
  Server(Server&&) = delete;
  Server& operator=(const Server&) = default;
  Server& operator=(Server&&) = delete;
  Server(cloe::ServerConf config) : config_(std::move(config)) {}
  virtual ~Server() = default;

  /**
   * Return the server configuration.
   */
  [[nodiscard]] const cloe::ServerConf& config() const { return config_; }

  /**
   * Return whether the server is alive and listening for requests.
   */
  [[nodiscard]] virtual bool is_listening() const = 0;

  /**
   * Return whether the server is currently streaming buffer data to a file.
   *
   * If it is, expect performance to be bad.
   */
  [[nodiscard]] virtual bool is_streaming() const = 0;

  /**
   * Start the web server.
   */
  virtual void start() = 0;

  /**
   * Stop all server-related procedures.
   */
  virtual void stop() = 0;

  /**
   * Open a file for api data streaming. This does not require a running web
   * server.
   */
  virtual void init_stream(const std::string& filename) = 0;

  /**
   * Register a list of all endpoints.
   */
  virtual void enroll(cloe::Registrar& r) = 0;

  /**
   * Return a new ServerRegistrar that lets you register static content and
   * API endpoints with the web server.
   */
  [[nodiscard]] virtual std::unique_ptr<ServerRegistrar> server_registrar() = 0;

  /**
   * Refresh and/or start streaming api data to a file.
   */
  virtual void refresh_buffer_start_stream() = 0;

  /**
   * Refresh and/or write api data to a file.
   */
  virtual void refresh_buffer() = 0;

  /**
   * Return a list of all registered endpoints.
   */
  [[nodiscard]] virtual std::vector<std::string> endpoints() const = 0;

  /**
   * Return a write lock guard on the server.
   *
   * Keep the returned value alive for as long as the server should be locked.
   * Once the destructor is called, the lock is released.
   *
   * \return Lock guard
   */
  [[nodiscard]] virtual Defer lock() = 0;

 protected:
  cloe::Logger logger() const { return cloe::logger::get("cloe"); }
  cloe::ServerConf config_;
};

/**
 * Create a new Server instance with the given configuration.
 */
std::unique_ptr<Server> make_server(const cloe::ServerConf&);

}  // namespace engine
