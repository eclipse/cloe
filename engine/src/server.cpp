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
 * \file server.cpp
 * \see  server.hpp
 *
 * This file is included in the build when the server component is enabled.
 */

#include "server.hpp"

#include <memory>   // for unique_ptr<>, make_unique
#include <utility>  // for make_pair

#include <cloe/registrar.hpp>                       // for HandlerType
#include <cloe/utility/output_serializer_json.hpp>  // for JsonFileSerializer

#include <oak/server.hpp>  // for Server, StaticRegistrar, ...

namespace engine {

class ServerRegistrarImpl : public ServerRegistrar {
 public:
  ServerRegistrarImpl(
      oak::Registrar static_reg, oak::ProxyRegistrar<cloe::HandlerType> api_reg)
      : static_registrar_(static_reg), api_registrar_(api_reg) {}

  std::unique_ptr<ServerRegistrar> clone() const override {
    return std::make_unique<ServerRegistrarImpl>(static_registrar_, api_registrar_);
  }

  std::unique_ptr<ServerRegistrar> with_prefix(const std::string& static_prefix,
                                               const std::string& api_prefix) const override {
    auto static_reg = static_registrar_;
    if (!static_prefix.empty()) {
      static_reg = static_registrar_.with_prefix(static_prefix);
    }
    auto api_reg = api_registrar_;
    if (!api_prefix.empty()) {
      api_reg = api_registrar_.with_prefix(api_prefix);
    }
    return std::make_unique<ServerRegistrarImpl>(static_reg, api_reg);
  }

  void register_static_handler(const std::string& endpoint, cloe::Handler h) override {
    static_registrar_.register_handler(endpoint, h);
  }

  void register_api_handler(const std::string& endpoint, cloe::HandlerType t,
                                    cloe::Handler h) override {
    api_registrar_.register_handler(endpoint, t, h);
  }

 private:
  oak::Registrar static_registrar_;
  oak::ProxyRegistrar<cloe::HandlerType> api_registrar_;
};

class ServerImpl : public Server {
 public:
  ServerImpl(const cloe::ServerConf& config) : Server(config) {
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

  ~ServerImpl() { stop(); }

 public:
  bool is_listening() const override { return server_.is_listening(); }

  bool is_streaming() const override { return is_streaming_; }

  void start() override {
    assert(!is_listening());

    logger()->info("Listening at: http://{}:{}", config_.listen_address, config_.listen_port);
    server_.set_address(config_.listen_address);
    server_.set_port(config_.listen_port);
    server_.set_threads(config_.listen_threads);
    server_.listen();
  }

  void init_stream(const std::string& filename) override {
    serializer_ = make_json_file_serializer(cloe::utility::JsonFileType::JSON_GZIP, logger());
    serializer_->open_file(filename);
  }

  void stop() override {
    if (is_listening()) {
      logger()->info("Stopping server...");
      server_.stop();
    }
    if (serializer_ != nullptr) {
      serializer_->close_file();
    }
  }

  void enroll(cloe::Registrar& r) override {
    r.register_api_handler(
        "/endpoints", cloe::HandlerType::STATIC,
        [this](const cloe::Request&, cloe::Response& r) { r.write(this->server_.endpoints()); });
  }

  std::unique_ptr<ServerRegistrar> server_registrar() override {
    return std::make_unique<ServerRegistrarImpl>(static_registrar(), api_registrar());
  }

  oak::Registrar static_registrar() { return static_registrar_.with("", nullptr); }

  oak::ProxyRegistrar<cloe::HandlerType> api_registrar() {
    return oak::ProxyRegistrar<cloe::HandlerType>({
        std::make_pair(cloe::HandlerType::STATIC, &static_api_registrar_),
        std::make_pair(cloe::HandlerType::DYNAMIC, &locked_api_registrar_),
        std::make_pair(cloe::HandlerType::BUFFERED, &buffer_api_registrar_),
    });
  }

  void refresh_buffer_start_stream() override {
    is_streaming_ = serializer_ != nullptr;
    if (is_listening() || is_streaming()) {
      buffer_api_registrar_.refresh_buffer();
    }
    if (is_streaming()) {
      // Write static endpoints at the beginning of the file.
      write_data_stream(static_api_registrar_.endpoints());
      write_data_stream(locked_api_registrar_.endpoints());
      write_data_stream(buffer_api_registrar_.endpoints());
    }
  }

  void refresh_buffer() override {
    if (is_listening() || is_streaming()) {
      buffer_api_registrar_.refresh_buffer();
    }
    if (is_streaming()) {
      write_data_stream(locked_api_registrar_.endpoints());
      write_data_stream(buffer_api_registrar_.endpoints());
    }
  }

  Defer lock() override {
    auto lock = locked_api_registrar_.lock();
    return Defer([&]() { lock.release(); });
  }

 private:
  void write_data_stream(const std::vector<std::string>& endpoints) const {
    auto j = server_.endpoints_to_json(endpoints);
    if (!j.empty()) {
      serializer_->serialize(j);
    }
  }

 private:
  oak::Server server_;
  oak::StaticRegistrar static_registrar_{&server_, config_.static_prefix, nullptr};
  oak::StaticRegistrar static_api_registrar_{&server_, config_.api_prefix, nullptr};
  oak::LockedRegistrar locked_api_registrar_{&server_, config_.api_prefix, nullptr};
  oak::BufferRegistrar buffer_api_registrar_{&server_, config_.api_prefix, nullptr};
  bool is_streaming_{false};
  std::unique_ptr<cloe::utility::JsonFileSerializer> serializer_;
};

std::unique_ptr<Server> make_server(const cloe::ServerConf& c) {
  return std::make_unique<ServerImpl>(c);
}

}  // namespace engine
