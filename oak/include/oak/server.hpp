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

#include <atomic>  // for atomic<>
#include <memory>  // for unique_ptr<>
#include <string>  // for string
#include <vector>  // for vector<>

#include <boost/asio.hpp>    // for boost::asio::io_context
#include <boost/thread.hpp>  // for boost::thread

#include "oak/registrar.hpp"    // for StaticRegistrar, BufferRegistrar
#include "oak/route_muxer.hpp"  // for Muxer<>

namespace oak {

/**
 * @brief Enumerates the state of the Server
 */
enum class ServerState {
  /**
   * @brief Server is in default-idle state
   */
  Default,
  /**
   * @brief Server initializes and is about to bind the address
   */
  Init,
  /**
   * @brief Server is listening with one or more threads
   */
  Listening,
  /**
   * @brief Server left listening state on one or more worker-threads
   */
  Stopping,
  /**
   * @brief Server stopped listening
   */
  Stopped,
};

/**
 * A Server accepts and serves endpoints for Handlers.
 *
 * This is a convenience wrapper around ServerImpl.
 */
class Server {
 public:
  /**
  * @brief Construct a new Server object
  *
  * @param addr Address to bind (e.g. 0.0.0.0, ::1, 127.0.0.1)
  * @param port TCP Port to bind
  */
  Server(const std::string& addr, unsigned short port)
      : listen_addr_(addr)
      , listen_port_(port)
      , listen_threads_(10)
      , state_{ServerState::Default}
      , ioc_(1)
      , acceptor_(ioc_)
      , socket_(ioc_) {
    init();
  }

  /**
   * @brief Construct a new Server on localhost:8080
   */
  Server() : Server("127.0.0.1", 8080) {}

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
   * Get the address on which the server listens
   */
  inline const auto& address() const { return listen_addr_; }
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
   * Get the port on which the server listens
   */
  inline const auto port() const { return listen_port_; }
  /**
   * Set the port on which to listen.
   */
  void set_port(int port) { listen_port_ = port; }

  /**
   * Returns whether the server has started and is currently listening.
   */
  bool is_listening() const { return state_.load() == ServerState::Listening; }

  /**
   * Start the server.
   */
  void listen();

  /**
   * Return endpoint data in json format.
   */
  cloe::Json endpoints_to_json(const std::vector<std::string>& endpoints) const;

  /**
   * Stop the server.
   */
  void stop();

  /**
   * Return a list of all registered endpoints.
   */
  inline std::vector<std::string> endpoints() const { return muxer_.routes(); }

 protected:
  friend StaticRegistrar;
  friend LockedRegistrar;
  friend BufferRegistrar;

  /**
   * Add a handler with the route muxer in the internal handler routine.
   */
  inline void add_handler(const std::string& key, Handler h) { muxer_.add(key, h); }

 private:
  void init();
  /**
   * @brief Asynchronously accepts connections via the socket utilizing the acceptor
   * @param acceptor Acceptor used for receiving connections
   * @param socket Socket used for communication
   */
  void serve(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket);
  /**
   * @brief Serves/Pumps the data
   * @note This method blocks until the server is stopped
   */
  void server_thread();
  /**
   * @brief Actual serving/pumping method used by server_thread & worker-threads
   */
  void server_thread_impl();

  /**
   * @brief IP-Address on which the server is listening (e.g. 0.0.0.0, ::1, 127.0.0.1)
   */
  std::string listen_addr_;
  /**
   * @brief TCP Port on which the server is listening
   */
  unsigned short listen_port_;
  /**
   * @brief
   */
  unsigned int listen_threads_;
  /**
   * @brief State of the server
   */
  std::atomic<ServerState> state_;
  /**
   * @brief Cloe endpoint multiplexer
   */
  Muxer<Handler> muxer_;
  /**
   * @brief Asio IOContext
   */
  boost::asio::io_context ioc_;
  /**
   * @brief The used Asio acceptor
   */
  boost::asio::ip::tcp::acceptor acceptor_;
  /**
   * @brief The used Asio socket
   */
  boost::asio::ip::tcp::socket socket_;
  /**
   * @brief Dedicated thread for server_thread()
   */
  std::thread thread_;
};

}  // namespace oak
