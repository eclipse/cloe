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

// Requires: cppnetlib
#include <boost/network/include/http/server.hpp>
#include <boost/network/utils/thread_pool.hpp>

#include "oak/registrar.hpp"    // for StaticRegistrar, BufferRegistrar
#include "oak/route_muxer.hpp"  // for Muxer<>

namespace oak {

// These forward-declarations are a necessary evil:
class ServerImplHandler;
using ServerImpl = boost::network::http::server<ServerImplHandler>;

/**
 * ServerImplHandler is the main request handler for the server.
 *
 * It implements an interface defined by boost::network::http::server.
 */
class ServerImplHandler {
 public:
  ServerImplHandler();

  /**
   * Handle every request from the server.
   *
   * Every single request that passes through the server has to go through this
   * handler. If the muxer has an endpoint that matches the request, then it
   * gets passed through. The muxer has a default endpoint, so the nominal case
   * is that every request is passed to some handler from the muxer.
   */
  void operator()(ServerImpl::request const&, ServerImpl::connection_ptr);

  /**
   * Log an error from the server.
   */
  void log(const ServerImpl::string_type& msg);

  /**
   * Add a handler for a specific endpoint.
   */
  void add(const std::string& key, Handler h);

  /**
   * Return a list of all registered endpoints.
   */
  std::vector<std::string> endpoints() const { return muxer.routes(); }

  /**
   * Return endpoint data in json format.
   */
  cloe::Json endpoints_to_json(const std::vector<std::string>& endpoints) const;

 private:
  Muxer<Handler> muxer;
};

/**
 * A Server accepts and serves endpoints for Handlers.
 *
 * This is a convenience wrapper around ServerImpl.
 */
class Server {
 public:
  Server(const std::string& addr, int port)
      : listen_addr_(addr), listen_port_(port), listen_threads_(3), listening_(false) {}

  Server() : listen_addr_("127.0.0.1"), listen_port_(8080), listen_threads_(3), listening_(false) {}

  /**
   * When a Server goes out of scope, it will stop listening for you if you
   * haven't done so already.
   */
  ~Server() {
    if (this->is_listening()) {
      this->stop();
    }
  }

  /**
   * Set the number of threads used for listening to connections.
   * This is the number of requests that can be handled simultaneously.
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

  /**
   * Set the port on which to listen.
   */
  void set_port(int port) { listen_port_ = port; }

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
  cloe::Json endpoints_to_json(const std::vector<std::string>& endpoints) const {
    return handler_.endpoints_to_json(endpoints);
  };

  /**
   * Stop the server.
   */
  void stop();

  /**
   * Return a list of all registered endpoints.
   */
  std::vector<std::string> endpoints() const { return handler_.endpoints(); }

 protected:
  friend StaticRegistrar;
  friend LockedRegistrar;
  friend BufferRegistrar;

  /**
   * Add a handler with the route muxer in the internal handler routine.
   */
  void add_handler(const std::string& key, Handler h) { handler_.add(key, h); }

 private:
  // Configuration
  std::string listen_addr_;
  int listen_port_;
  int listen_threads_;

  // State
  bool listening_;
  ServerImplHandler handler_;
  std::unique_ptr<ServerImpl::options> options_;
  std::unique_ptr<ServerImpl> server_;
  std::unique_ptr<boost::thread> thread_;
};

}  // namespace oak
