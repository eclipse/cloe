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
 * \file registrar.hpp
 */

#pragma once

#include <memory>       // for unique_ptr<>, shared_ptr<>
#include <string>       // for string
#include <string_view>  // for string_view

#include <cloe/core/logger.hpp>   // for logger::get
#include <cloe/registrar.hpp>     // for cloe::Registrar
#include <cloe/stack_config.hpp>  // for CLOE_TRIGGER_PATH_DELIMITER, ...

#include "coordinator.hpp"  // for Coordinator
#include "server.hpp"       // for Server, ServerRegistrar

namespace engine {

class Registrar : public cloe::Registrar {
 public:
  Registrar(std::unique_ptr<ServerRegistrar> r, Coordinator* c, cloe::DataBroker* db)
      : server_registrar_(std::move(r)), coordinator_(c), data_broker_(db) {}

  Registrar(const Registrar& ar,
            const std::string& trigger_prefix,
            const std::string& static_prefix,
            const std::string& api_prefix)
      : coordinator_(ar.coordinator_), data_broker_(ar.data_broker_) {
    if (trigger_prefix.empty()) {
      trigger_prefix_ = ar.trigger_prefix_;
    } else {
      trigger_prefix_ = ar.trigger_prefix_ + trigger_prefix;
    }
    server_registrar_ = ar.server_registrar_->with_prefix(static_prefix, api_prefix);
  }

  void register_static_handler(const std::string& endpoint, cloe::Handler h) override {
    server_registrar_->register_static_handler(endpoint, h);
  }

  void register_api_handler(const std::string& endpoint,
                            cloe::HandlerType t,
                            cloe::Handler h) override {
    server_registrar_->register_api_handler(endpoint, t, h);
  }

  [[nodiscard]] std::unique_ptr<cloe::Registrar> clone() const {
    return std::make_unique<Registrar>(*this, "", "", "");
  }

  std::unique_ptr<cloe::Registrar> with_static_prefix(const std::string& prefix) const override {
    assert(!prefix.empty());
    return std::make_unique<Registrar>(*this, "", prefix, "");
  }

  std::unique_ptr<cloe::Registrar> with_api_prefix(const std::string& prefix) const override {
    assert(!prefix.empty());
    return std::make_unique<Registrar>(*this, "", "", prefix);
  }

  std::unique_ptr<cloe::Registrar> with_trigger_prefix(const std::string& prefix) const override {
    assert(!prefix.empty() && prefix[0] != '_');
    return std::make_unique<Registrar>(*this, prefix, "", "");
  }

  [[nodiscard]] std::string make_prefix(std::string_view name, std::string_view delim) const {
    assert(!name.empty());

    if (trigger_prefix_.empty()) {
      // This only works for Cloe internal triggers.
      return std::string(name);
    }

    std::string prefix = trigger_prefix_;
    if (name == "_") {
      // Special case: "_" means we can actually use just trigger_prefix_.
      // This might cause a problem if we name a plugin the same as one
      // of the internal Cloe triggers...
      return prefix;
    }
    prefix += delim;
    prefix += name;
    return prefix;
  }

  [[nodiscard]] std::string make_trigger_name(std::string_view name) const {
    return make_prefix(name, CLOE_TRIGGER_PATH_DELIMITER);
  }

  [[nodiscard]] std::string make_signal_name(std::string_view name) const override {
    auto sname = make_prefix(name, CLOE_SIGNAL_PATH_DELIMITER);
    coordinator_->logger()->debug("Register signal: {}", sname);
    return sname;
  }

  void register_action(cloe::ActionFactoryPtr&& af) override {
    coordinator_->register_action(make_trigger_name(af->name()), std::move(af));
  }

  void register_event(
      cloe::EventFactoryPtr&& ef, std::shared_ptr<cloe::Callback> storage) override {
    coordinator_->register_event(make_trigger_name(ef->name()), std::move(ef), storage);
  }

  sol::table register_lua_table() override {
    return coordinator_->register_lua_table(trigger_prefix_);
  }

  [[nodiscard]] cloe::DataBroker& data_broker() const override {
    assert(data_broker_ != nullptr);
    return *data_broker_;
  }

 private:
  std::unique_ptr<ServerRegistrar> server_registrar_;
  Coordinator* coordinator_;       // non-owning
  cloe::DataBroker* data_broker_;  // non-owning
  std::string trigger_prefix_;
};

}  // namespace engine
