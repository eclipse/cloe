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
 * \file server.hpp
 */

#pragma once

#include <utility>  // for make_pair

#include <cloe/handler.hpp>    // for Request, Response
#include <cloe/registrar.hpp>  // for Registrar

#include "stack.hpp"      // for ServerConf
#include "oak/server.hpp"  // for Server, StaticRegistrar, ...

namespace engine {

class Server {
 public:  // Configuration
  const cloe::ServerConf config;

 public:  // Construction & Destruction
  Server(const cloe::ServerConf& c) : config(c) {
    auto rl = logger();

    // clang-format off
    static_registrar_.set_prefix(config.static_prefix);
    static_registrar_.set_logger([rl] (auto endpoint) {
        rl->debug("Register static endpoint:   {}", endpoint);
    });

    static_api_registrar_.set_prefix(config.api_prefix);
    static_api_registrar_.set_logger([rl] (auto endpoint) {
        rl->debug("Register static endpoint:   {}", endpoint);
    });

    locked_api_registrar_.set_prefix(config.api_prefix);
    locked_api_registrar_.set_logger([rl] (auto endpoint) {
        rl->debug("Register dynamic endpoint:  {}", endpoint);
    });

    buffer_api_registrar_.set_prefix(config.api_prefix);
    buffer_api_registrar_.set_logger([rl] (auto endpoint) {
        rl->debug("Register buffered endpoint: {}", endpoint);
    });
    // clang-format on
  }

  ~Server() { stop(); }

 public:
  bool is_listening() { return server_.is_listening(); }

  void start() {
    assert(!is_listening());

    logger()->info("Listening at: http://{}:{}", config.listen_address, config.listen_port);
    server_.set_address(config.listen_address);
    server_.set_port(config.listen_port);
    server_.set_threads(config.listen_threads);
    server_.listen();
  }

  void stop() {
    if (is_listening()) {
      logger()->info("Stopping server...");
      server_.stop();
    }
  }

  void enroll(cloe::Registrar& r) {
    r.register_api_handler(
        "/endpoints", cloe::HandlerType::STATIC,
        [this](const cloe::Request&, cloe::Response& r) { r.write(this->server_.endpoints()); });
  }

  /**
   * Return the static content registrar.
   */
  oak::Registrar static_registrar() const { return static_registrar_.with("", nullptr); }

  /**
   * Return the API registrar.
   */
  oak::ProxyRegistrar<cloe::HandlerType> api_registrar() {
    return oak::ProxyRegistrar<cloe::HandlerType>({
        std::make_pair(cloe::HandlerType::STATIC, &static_api_registrar_),
        std::make_pair(cloe::HandlerType::DYNAMIC, &locked_api_registrar_),
        std::make_pair(cloe::HandlerType::BUFFERED, &buffer_api_registrar_),
    });
  }

  /**
   * Refresh the server buffer.
   */
  void refresh_buffer() {
    if (config.listen) {
      buffer_api_registrar_.refresh_buffer();
    }
  }

  /**
   * Return a write lock on the server.
   */
  boost::unique_lock<boost::shared_mutex> lock() { return locked_api_registrar_.lock(); }

 protected:
  cloe::Logger logger() const { return cloe::logger::get("cloe"); }

 private:  // State
  oak::Server server_;
  oak::StaticRegistrar static_registrar_{&server_, config.static_prefix, nullptr};
  oak::StaticRegistrar static_api_registrar_{&server_, config.api_prefix, nullptr};
  oak::LockedRegistrar locked_api_registrar_{&server_, config.api_prefix, nullptr};
  oak::BufferRegistrar buffer_api_registrar_{&server_, config.api_prefix, nullptr};
};

}  // namespace engine
