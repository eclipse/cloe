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
 * \file server_mock.cpp
 * \see  server.hpp
 * \see  server.cpp
 *
 * This file is included in the build when the server component is disabled.
 * This allows us to decouple the engine from the oak dependency with all
 * it entails in one place.
 *
 * We still need to implement the interfaces defined in server.hpp though.
 *
 * Having a separate file makes it easier to diff the two implmentation files.
 */

#include "server.hpp"

#include <memory>   // for unique_ptr<>, make_unique

#include <cloe/registrar.hpp>                       // for HandlerType

namespace engine {

class ServerRegistrarImpl : public ServerRegistrar {
 public:
  ServerRegistrarImpl(const std::string& static_prefix, const std::string& api_prefix)
    : static_prefix_(static_prefix), api_prefix_(api_prefix) {}

  std::unique_ptr<ServerRegistrar> clone() const override {
    return std::make_unique<ServerRegistrarImpl>(static_prefix_, api_prefix_);
  }

  std::unique_ptr<ServerRegistrar> with_prefix(const std::string& static_prefix,
                                               const std::string& api_prefix) const override {
    assert(static_prefix.empty() || static_prefix[0] == '/');
    assert(api_prefix.empty() || api_prefix[0] == '/');
    return std::make_unique<ServerRegistrarImpl>(static_prefix_ + static_prefix,
                                                 api_prefix_ + api_prefix);
  }

  void register_static_handler(const std::string& endpoint, cloe::Handler) override {
    logger()->warn("Unregistered static endpoint:   {}", endpoint);
  }

  void register_api_handler(const std::string& endpoint, cloe::HandlerType t,
                                    cloe::Handler) override {
    switch (t) {
      case cloe::HandlerType::STATIC:
        logger()->warn("Unregistered static endpoint:   {}", endpoint);
        break;
      case cloe::HandlerType::DYNAMIC:
        logger()->warn("Unregistered dynamic endpoint:  {}", endpoint);
        break;
      case cloe::HandlerType::BUFFERED:
        logger()->warn("Unregistered buffered endpoint: {}", endpoint);
        break;
      default:
        throw std::runtime_error("unknown handler type");
    }
  }

  cloe::Logger logger() const {
    return cloe::logger::get("cloe");
  }

 private:
  std::string static_prefix_{"/"};
  std::string api_prefix_{"/"};
};

class ServerImpl : public Server {
 public:
  ServerImpl(const cloe::ServerConf& config) : Server(config) {
    if (config.listen) {
      logger()->error("Server unavailable, but configuration value /server/listen = true");
      logger()->error("Server unavailable, feature is not compiled into engine.");
    }
  }

  ~ServerImpl() {}

 public:
  bool is_listening() const override { return false; }

  bool is_streaming() const override { return false; }

  void start() override {
    logger()->error("Server unavailable, cannot start.");
  }

  void init_stream(const std::string& ) override {
    logger()->error("Server unavailable, cannot initialize stream.");
  }

  void stop() override {
  }

  void enroll(cloe::Registrar& ) override {
  }

  std::unique_ptr<ServerRegistrar> server_registrar() override {
    return server_registrar_.clone();
  }

  void refresh_buffer_start_stream() override {
  }

  void refresh_buffer() override {
  }

  std::vector<std::string> endpoints() const override {
    return {};
  }

  Defer lock() override {
    return Defer([]() {});
  }

 private:
  ServerRegistrarImpl server_registrar_{ "", "" };
};

std::unique_ptr<Server> make_server(const cloe::ServerConf& c) {
  return std::make_unique<ServerImpl>(c);
}

} // namespace engine
