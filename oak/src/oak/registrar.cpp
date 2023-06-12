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
 * \file oak/registrar.cpp
 * \see  oak/registrar.hpp
 */

#include "oak/registrar.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

#include <cloe/handler.hpp>  // for Request, Response, Handler

#include "oak/request_stub.hpp"  // for RequestStub
#include "oak/server.hpp"        // for Server

namespace oak {

Middleware chain_middleware(Middleware x, Middleware y) {
  if (x == nullptr) {
    return y;
  } else if (y == nullptr) {
    return x;
  } else {
    return [x, y](cloe::Handler h) -> cloe::Handler { return y(x(h)); };
  }
}

Registrar::Registrar(const Registrar* r, const std::string& prefix, Middleware m)
    : proxy_(const_cast<Registrar*>(r)), prefix_(prefix), middleware_(m) {
  assert(r != nullptr);
  assert(prefix.size() == 0 || prefix[0] == '/');
}

Registrar Registrar::with(const std::string& prefix, Middleware m) const {
  assert(prefix.size() == 0 || prefix[0] == '/');
  return Registrar(this, prefix_ + prefix, chain_middleware(middleware_, m));
}

Registrar Registrar::with_prefix(const std::string& prefix) const {
  assert(prefix.size() > 0 && prefix[0] == '/');
  return Registrar(this, prefix_ + prefix, middleware_);
}

Registrar Registrar::with_middleware(Middleware m) const {
  assert(m != nullptr);
  return Registrar(this, prefix_, chain_middleware(middleware_, m));
}

void Registrar::register_handler(const std::string& route, cloe::Handler h) {
  assert(route.size() != 0 && route[0] == '/');
  assert(proxy_ != nullptr);
  if (middleware_) {
    h = middleware_(h);
  }
  auto endpoint = route;
  if (prefix_.size() != 0) {
    endpoint = prefix_ + route;
  }
  proxy_->register_handler(endpoint, h);
}

void StaticRegistrar::register_handler(const std::string& route, cloe::Handler h) {
  assert(route.size() != 0 && route[0] == '/');
  assert(proxy_ == nullptr);
  auto endpoint = prefix_ + route;
  log(endpoint);
  if (middleware_) {
    server_->add_handler(endpoint, middleware_(h));
  } else {
    server_->add_handler(endpoint, h);
  }
  endpoints_.push_back(endpoint);
}

void LockedRegistrar::register_handler(const std::string& route, cloe::Handler h) {
  assert(route.size() != 0 && route[0] == '/');
  assert(proxy_ == nullptr);
  h = [this, h](const cloe::Request& q, cloe::Response& r) {
    std::shared_lock read_lock(this->access_);
    h(q, r);
  };

  StaticRegistrar::register_handler(route, h);
}

void BufferRegistrar::register_handler(const std::string& route, cloe::Handler h) {
  assert(route.size() != 0 && route[0] == '/');
  assert(proxy_ == nullptr);
  auto key = Muxer<cloe::Handler>::normalize(prefix_ + route);
  log(key);
  if (middleware_) {
    handlers_.add(key, middleware_(h));
  } else {
    handlers_.add(key, h);
  }
  this->endpoints_.push_back(key);
  // Since it's not available to the server yet, we don't need to
  // lock for refreshing the route.
  refresh_route(key);
  server_->add_handler(key, [this, key](const cloe::Request&, cloe::Response& r) {
    // Technically it's not necessary to lock, but when we are updating the
    // buffers, we do not want any requests to get through.
    std::shared_lock read_lock(this->access_);
    r = this->buffer_.get(key).first;
  });
}

void BufferRegistrar::refresh_buffer() {
  std::unique_lock write_lock(access_);
  // We don't delete handlers, so there will never be routes
  // that are in handlers_ but not in buffer_.
  for (auto key : handlers_.routes()) {
    refresh_route(key);
  }
}

void BufferRegistrar::refresh_route(const std::string& key) {
  const RequestStub q;
  auto handler = handlers_.get_unsafe(key).first;
  cloe::Response r;
  handler(q, r);
  buffer_.set_unsafe(key, r);
}

}  // namespace oak
