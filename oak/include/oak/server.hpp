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
 * \file oak/server.hpp
 * \see  oak/server.cpp
 */

#pragma once

#include <memory>  // for unique_ptr<>
#include <string>  // for string
#include <vector>  // for vector<>

#include <cloe/cloe_fwd.hpp>         // for Handler
#include <fable/fable_fwd.hpp>       // for Json
#include <oatpp/network/Server.hpp>  // for Server

#include "oak/registrar.hpp"  // for StaticRegistrar, BufferRegistrar

namespace oak {

class GreedyHandler;

/**
 * A Server accepts and serves endpoints for Handlers.
 *
 * This is a convenience wrapper around ServerImpl.
 */
class Server {
 public:
  Server(const std::string& addr, int port);
  Server();

  /**
   * When a Server goes out of scope, it will stop listening for you if you
   * haven't done so already.
   */
  ~Server();

  /**
   * Set the number of threads used for listening to connections.
   * This is the number of requests that can be handled simultaneously.
   *
   * NOTE: This doesn't do anything at the moment; a new thread is created
   * for each connection.
   */
  void set_threads(int n) { listen_threads_ = n; }

  /**
   * Set the address on which the server will listen.
   *
   * - Use 127.0.0.1 to only allow local connections.
   * - Use 0.0.0.0 to listen on all interfaces. This will allow clients from
   *   the entire network to access this service, if the system firewall allows
   *   it.
   */
  void set_address(const std::string& addr) { listen_addr_ = addr; }

  const std::string& address() const { return listen_addr_; }

  /**
   * Set the port on which to listen.
   */
  void set_port(int port) { listen_port_ = port; }

  int port() const { return listen_port_; }

  /**
   * Returns whether the server has started and is currently listening.
   */
  bool is_listening() const { return listening_; }

  /**
   * Start the server.
   */
  void listen();

  /**
   * Return endpoint data in json format.
   */
  fable::Json endpoints_to_json(const std::vector<std::string>& endpoints) const;

  /**
   * Stop the server.
   */
  void stop();

  /**
   * Return a list of all registered endpoints.
   */
  std::vector<std::string> endpoints() const;

 protected:
  friend StaticRegistrar;
  friend LockedRegistrar;
  friend BufferRegistrar;

  /**
   * Add a handler with the route muxer in the internal handler routine.
   */
  void add_handler(const std::string& key, cloe::Handler h);

 private:
  // Configuration
  std::string listen_addr_;
  int listen_port_;
  int listen_threads_;

  // State
  bool listening_;
  std::shared_ptr<oatpp::network::Server> server_;
  std::shared_ptr<GreedyHandler> handler_;
};

}  // namespace oak
