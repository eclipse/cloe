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
 * \file oak/registrar.hpp
 * \see  oak/registrar.cpp
 */

#pragma once

#include <functional>
#include <initializer_list>
#include <map>
#include <mutex>
#include <string>
#include <utility>

#include <cloe/handler.hpp>  // for Handler, Response

#include "oak/route_muxer.hpp"  // for Muxer

namespace oak {

class Server; // from oak/server.hpp

using Middleware = std::function<cloe::Handler(cloe::Handler)>;

using Logger = std::function<void(const std::string&)>;

/**
 * Return a Middleware that performs x then y on incoming handlers.
 *
 * This is effectively:
 *
 *    return y(x(h));
 */
Middleware chain_middleware(Middleware x, Middleware y);

/**
 * Registrar is the interface around the act of registering a handler safely.
 *
 * This class in particular acts also as a proxy.
 *
 * Warning: setting various middleware can have different effects
 * depending on which registrar you are using. Consider it unsafe.
 */
class Registrar {
 public:
  Registrar(const Registrar* r, const std::string& prefix, Middleware m);
  virtual ~Registrar() = default;

  Registrar with(const std::string& prefix, Middleware m) const;
  Registrar with_prefix(const std::string& prefix) const;
  Registrar with_middleware(Middleware m) const;
  std::string prefix() const { return prefix_; }
  Middleware middleware() const { return middleware_; }
  virtual void register_handler(const std::string& route, cloe::Handler h);

 protected:
  Registrar(const std::string& prefix, Middleware m) : prefix_(prefix), middleware_(m) {}

 protected:
  Registrar* proxy_{nullptr};
  std::string prefix_;
  Middleware middleware_;
};

template <typename T>
class ProxyRegistrar {
 public:
  ProxyRegistrar(const std::map<T, Registrar*>& registrars, const std::string& prefix, Middleware m)
      : registrars_(registrars), prefix_(prefix), middleware_(m) {}
  explicit ProxyRegistrar(std::initializer_list<std::pair<T, Registrar*>> registrars) {
    for (auto p : registrars) {
      registrars_[p.first] = p.second;
    }
  }

  ProxyRegistrar<T> with(const std::string& prefix, Middleware m) const {
    assert(prefix.size() == 0 || prefix[0] == '/');
    return ProxyRegistrar<T>(registrars_, prefix_ + prefix, chain_middleware(middleware_, m));
  }

  ProxyRegistrar<T> with_prefix(const std::string& prefix) const {
    assert(prefix.size() > 0 && prefix[0] == '/');
    return ProxyRegistrar<T>(registrars_, prefix_ + prefix, middleware_);
  }

  ProxyRegistrar<T> with_middleware(Middleware m) const {
    assert(m != nullptr);
    return ProxyRegistrar<T>(registrars_, prefix_, chain_middleware(middleware_, m));
  }

  std::string prefix() const { return prefix_; }
  Middleware middleware() const { return middleware_; }

  void register_handler(const std::string& route, T select, cloe::Handler h) {
    assert(route.size() != 0 && route[0] == '/');
    assert(registrars_.size() != 0);
    if (middleware_) {
      h = middleware_(h);
    }
    auto endpoint = route;
    if (prefix_.size() != 0) {
      endpoint = prefix_ + route;
    }
    registrars_.at(select)->register_handler(endpoint, h);
  }

 private:
  std::map<T, Registrar*> registrars_;
  std::string prefix_;
  Middleware middleware_;
};

/**
 * StaticRegistrar provides a registrar implementation that is safe for static
 * content handlers.
 *
 * The contract requires that only handlers are registered that access data
 * that never changes or manage the safety themselves.
 */
class StaticRegistrar : public Registrar {
 public:
  explicit StaticRegistrar(Server* s) : Registrar("", nullptr), server_(s) {
    assert(server_ != nullptr);
  }
  StaticRegistrar(Server* s, const std::string& prefix, Middleware m)
      : Registrar(prefix, m), server_(s) {
    assert(server_ != nullptr);
    assert(prefix_.size() == 0 || prefix_[0] == '/');
  }
  virtual ~StaticRegistrar() = default;

  void register_handler(const std::string& route, cloe::Handler h) override;

  void set_prefix(const std::string& prefix) { prefix_ = prefix; }
  void set_logger(Logger logger) { logger_ = logger; }

  const std::vector<std::string>& endpoints() const { return endpoints_; }

 protected:
  void log(const std::string& endpoint) {
    if (logger_) {
      logger_(endpoint);
    }
  }

 protected:
  Server* server_;
  Logger logger_;
  std::vector<std::string> endpoints_;
};

/**
 * LockedRegistrar provides a registrar implementation that is safe for
 * dynamically changing data content handlers.
 *
 * The contract requires a write lock to be acquired before changing any
 * of the data that might be access from handlers added.
 * When the write lock is held, all requests are blocked to avoid data
 * races.
 *
 * The throughput of requests is strongly limited by this registrar.
 */
class LockedRegistrar : public StaticRegistrar {
 public:
  using StaticRegistrar::StaticRegistrar;

  void register_handler(const std::string& route, cloe::Handler h) override;

  /**
   * Return a unique lock guard so that the backing data can be modified.
   *
   * On destruction, the lock is released.
   */
  std::unique_lock<std::shared_mutex> lock() {
    return std::unique_lock(access_);
  }

 private:
  mutable std::shared_mutex access_;
};

/**
 * BufferRegistrar provides a performant registrar implementation that is safe
 * for dynamically change data content handlers.
 *
 * The great problem that the locked registrar has is performance. In general,
 * one anyway only wants to receive data from different endpoints that is
 * coherent. By creating a buffer we pay the overhead of creating a response
 * pre-emptively for each endpoint with the advantage that this response can
 * be returned even while the data from which the response came is changing.
 * There is still a portion of time when the registrar is locked, but this
 * is limited to the time it takes to refresh the buffer.
 *
 * The major downside of the BufferRegistrar is that responses cannot incur
 * any side-effects and may not use the request data.
 *
 * The contract requires that refresh_buffer be called whenever updated data
 * should be made available.
 */
class BufferRegistrar : public StaticRegistrar {
 public:
  using StaticRegistrar::StaticRegistrar;

  /**
   * Do not register handlers that want to make use of Request.
   */
  void register_handler(const std::string& route, cloe::Handler h) override;

  /**
   * Refresh the entire buffer by calling every single registered
   * handler once.
   *
   * During the refresh, no endpoints that belong to the Registrar
   * will be accessed.
   */
  void refresh_buffer();

 protected:
  /**
   * Refresh the buffer for the given route.
   *
   * This should only occur with a write lock enabled or if the
   * route is not yet available to the server.
   */
  void refresh_route(const std::string& key);

 protected:
  mutable std::shared_mutex access_;
  Muxer<cloe::Response> buffer_;
  Muxer<cloe::Handler> handlers_;
};

}  // namespace oak
