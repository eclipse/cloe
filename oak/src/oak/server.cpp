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
 * \file oak/server.cpp
 * \see  oak/server.hpp
 */

#include "oak/server.hpp"

#include <condition_variable>  // for condition_variable
#include <functional>          // for bind
#include <map>                 // for map<>
#include <memory>              // for make_shared<>
#include <mutex>               // for mutex, unique_lock
#include <sstream>             // for stringstream
#include <string>              // for string

#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

#include <cloe/core/logger.hpp>  // for logger::get
#include <cloe/handler.hpp>      // for Request
#include <fable/json.hpp>        // for Json
using namespace cloe;            // NOLINT(build/namespaces)

#include "oak/request_stub.hpp"  // for RequestStub
#include "oak/route_muxer.hpp"   // for Muxer<>

namespace oak {

/**
 * Implementation of cloe::Request interface.
 */
class Request : public cloe::Request {
 private:
  std::string dest_;
  std::string endpoint_;
  std::map<std::string, std::string> queries_;
  std::string body_;
  RequestMethod method_;

 public:
  /**
   * Create a new Request from ServerImpl.
   *
   * - This should not panic.
   */
  explicit Request(const oatpp::web::protocol::http::incoming::Request& req) {
    auto head = req.getStartingLine();
    dest_ = head.path.std_str();
    endpoint_ = Muxer<cloe::Handler>::normalize(dest_);
    for (const auto& [k, v] : req.getQueryParameters().getAll()) {
      queries_[k.std_str()] = v.std_str();
    }
    from_string(head.method.std_str(), method_);
    if (method_ == RequestMethod::POST) {
      body_ = req.readBodyToString();
    }
  }

  RequestMethod method() const override { return method_; }
  ContentType type() const override { return ContentType::UNKNOWN; }
  const std::string& body() const override { return body_; }
  const std::string& uri() const override { return dest_; }
  const std::string& endpoint() const override { return endpoint_; }
  const std::map<std::string, std::string>& query_map() const override { return queries_; }
};

/**
 * The GreedyHandler is a stop-gap till we refactor the server code.
 *
 * Instead of it handling a single route, it handles *all* routes,
 * hence the name.
 *
 * HttpRequestHandler defines several typedefs which are used within
 * this class, such as `OutgoingResponse` and `IncomingRequest`.
 */
class GreedyHandler : public oatpp::web::server::HttpRequestHandler {
 public:
  GreedyHandler() {
    muxer.set_default([this](const cloe::Request& q, cloe::Response& r) {
      logger()->debug("404 {}", q.endpoint());
      r.not_found(fable::Json{
          {"error", "cannot find handler"},
          {"endpoints", fable::Json(this->muxer.routes())},
      });
    });
  }

  /**
   * Handle every request from the server.
   *
   * Every single request that passes through the server has to go through this
   * handler. If the muxer has an endpoint that matches the request, then it
   * gets passed through. The muxer has a default endpoint, so the nominal case
   * is that every request is passed to some handler from the muxer.
   */
  std::shared_ptr<OutgoingResponse> handle(
      const std::shared_ptr<IncomingRequest>& request) override {
    auto to_response_impl = [](const cloe::Response& r) -> std::shared_ptr<OutgoingResponse> {
      auto code = Status(static_cast<int>(r.status()), "");
      auto type = cloe::as_cstr(r.type());

      auto out = ResponseFactory::createResponse(code, r.body());
      out->putOrReplaceHeader("Access-Control-Allow-Origin", "*");
      out->putOrReplaceHeader("Content-Type", type);
      return out;
    };

    try {
      Request q(*request);
      logger()->debug("{} {}", as_cstr(q.method()), q.endpoint());
      cloe::Response r;
      muxer.get(q.endpoint()).first(q, r);
      return to_response_impl(r);
    } catch (const std::exception& e) {
      cloe::Response err;
      err.error(StatusCode::SERVER_ERROR, std::string(e.what()));
      return to_response_impl(err);
    } catch (...) {
      cloe::Response err;
      err.error(StatusCode::SERVER_ERROR, std::string("unknown error occurred"));
      return to_response_impl(err);
    }
  }

  /**
   * Add a handler for a specific endpoint.
   */
  void add(const std::string& key, cloe::Handler h) { muxer.add(key, h); }

  /**
   * Return a list of all registered endpoints.
   */
  std::vector<std::string> endpoints() const { return muxer.routes(); }

  /**
   * Return endpoint data in json format.
   */
  fable::Json endpoints_to_json(const std::vector<std::string>& endpoints) const {
    fable::Json j;
    for (const auto& endpoint : endpoints) {
      const RequestStub q;
      cloe::Response r;
      try {
        muxer.get(endpoint).first(q, r);
      } catch (std::logic_error& e) {
        // Silently ignore endpoints that require an implementation of any of
        // the Request's methods.
        continue;
      }
      if (r.status() == cloe::StatusCode::OK && r.type() == cloe::ContentType::JSON) {
        j[endpoint] = fable::Json(r.body());
      }
    }
    return j;
  }

 private:
  cloe::Logger logger() { return cloe::logger::get("cloe-server"); }

 private:
  Muxer<cloe::Handler> muxer;
};

void Server::listen() {
  if (listening_) {
    throw std::runtime_error("already listening");
  }
  listening_ = true;

  oatpp::base::Environment::init();

  auto router = oatpp::web::server::HttpRouter::createShared();
  router->route("GET", "/*", handler_);
  router->route("POST", "/*", handler_);
  router->route("PUT", "/*", handler_);
  router->route("DELETE", "/*", handler_);

  auto handler = oatpp::web::server::HttpConnectionHandler::createShared(router);
  auto provider = oatpp::network::tcp::server::ConnectionProvider::createShared(
      {listen_addr_, static_cast<v_uint16>(listen_port_), oatpp::network::Address::IP_4});

  server_ = oatpp::network::Server::createShared(provider, handler);
  server_->run(true);
}

void Server::stop() {
  if (!listening_) {
    throw std::runtime_error("not listening");
  }

  server_ = nullptr;

  oatpp::base::Environment::destroy();

  listening_ = false;
}

void Server::add_handler(const std::string& key, cloe::Handler h) { handler_->add(key, std::move(h)); }

std::vector<std::string> Server::endpoints() const { return handler_->endpoints(); }

fable::Json Server::endpoints_to_json(const std::vector<std::string>& endpoints) const {
  return handler_->endpoints_to_json(endpoints);
}

Server::Server(const std::string& addr, int port)
    : listen_addr_(addr)
    , listen_port_(port)
    , listen_threads_(3)
    , listening_(false)
    , handler_(new GreedyHandler()) {}

Server::Server()
    : listen_addr_("127.0.0.1")
    , listen_port_(8080)
    , listen_threads_(3)
    , listening_(false)
    , handler_(new GreedyHandler()) {}

Server::~Server() {
  if (this->is_listening()) {
    this->stop();
  }
}

}  // namespace oak
