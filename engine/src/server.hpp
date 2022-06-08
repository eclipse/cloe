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
#include <oak/registrar.hpp>   // for oak::Registrar, oak::ProxyRegistrar

#include "stack.hpp"          // for ServerConf
#include "utility/defer.hpp"  // for Defer

namespace engine {

/**
 * Server interface to make altering the implementation easier.
 *
 * Use `make_server()` to create an instance that you can use.
 */
class Server {
 public:
  Server(const cloe::ServerConf& config) : config_(config) {}
  virtual ~Server() = default;

  /**
   * Return the server configuration.
   */
  const cloe::ServerConf& config() const { return config_; }

  /**
   * Return whether the server is alive and listening for requests.
   */
  virtual bool is_listening() const = 0;

  /**
   * Return whether the server is currently streaming buffer data to a file.
   *
   * If it is, expect performance to be bad.
   */
  virtual bool is_streaming() const = 0;

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
   * Return the static content registrar.
   */
  virtual oak::Registrar static_registrar() = 0;

  /**
   * Return the API registrar.
   */
  virtual oak::ProxyRegistrar<cloe::HandlerType> api_registrar() = 0;

  /**
   * Refresh and/or start streaming api data to a file.
   */
  virtual void refresh_buffer_start_stream() = 0;

  /**
   * Refresh and/or write api data to a file.
   */
  virtual void refresh_buffer() = 0;

  /**
   * Return a write lock guard on the server.
   *
   * Keep the returned value alive for as long as the server should be locked.
   * Once the destructor is called, the lock is released.
   *
   * \return Lock guard
   */
  virtual Defer lock() = 0;

 protected:
  cloe::Logger logger() const { return cloe::logger::get("cloe"); }
  cloe::ServerConf config_;
};

/**
 * Create a new Server instance with the given configuration.
 */
std::unique_ptr<Server> make_server(const cloe::ServerConf&);

}  // namespace engine
