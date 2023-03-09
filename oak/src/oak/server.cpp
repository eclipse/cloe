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

#include <boost/tokenizer.hpp>  // for tokenizer, char_separator

// Requires: boost-wannabe cppnetlib
#include <boost/network/include/http/server.hpp>
#include <boost/network/utils/thread_pool.hpp>

#include <cloe/core.hpp>     // for logger::get
#include <cloe/handler.hpp>  // for Request
using namespace cloe;        // NOLINT(build/namespaces)

#include "oak/request_stub.hpp"  // for RequestStub

namespace oak {

namespace {

cloe::Logger log() { return cloe::logger::get("cloe-server"); }

std::map<std::string, std::string> parse_queries(std::string dest) {
  std::map<std::string, std::string> queries;

  size_t pos = dest.find("?");
  if (pos != std::string::npos) {
    dest = dest.substr(pos + 1);

    // At this point we have a bunch of key-value pairs delimited by
    // '&'. Within each key-value pair, we split by '='.
    boost::char_separator<char> sep("&");
    boost::tokenizer<boost::char_separator<char>> tokens(dest, sep);
    for (const auto& kv : tokens) {
      size_t pos = kv.find("=");
      queries[kv.substr(0, pos)] = kv.substr(pos + 1);
    }
  }

  return queries;
}

/**
 * DataReceiver provides async and sync reading body from a request.
 * This needs to be implemented here because cpp-netlib does not support it.
 *
 * Much of the code here is based on the async_server_file_upload.cpp
 * example that is part of the 0.13 cpp-netlib release.
 */
template <typename T>
class DataReceiver {
  const typename T::request& req_;
  typename T::connection_ptr conn_;

  size_t content_length_{0};
  std::string data_{};
  std::mutex mutex_{};
  std::condition_variable condvar_{};

 public:
  DataReceiver(const typename T::request& req, const typename T::connection_ptr& conn)
      : req_(req), conn_(conn) {
    for (auto item : req.headers) {
      if (boost::to_lower_copy(item.name) == "content-length") {
        content_length_ = std::stoul(item.value);
        break;
      }
    }
  }

  std::string read_all() {
    if (content_length_ > 0) {
      std::unique_lock<std::mutex> guard{mutex_};
      read_chunk(conn_);
      condvar_.wait(guard);
    }
    return std::move(data_);
  }

 private:
  void read_chunk(typename T::connection_ptr conn) {
    conn->read(std::bind(&DataReceiver<T>::on_data_ready, this, std::placeholders::_1,
                         std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  }

  void on_data_ready(typename T::connection::input_range chunk,
                     boost::system::error_code error,
                     size_t chunk_size,
                     typename T::connection_ptr conn) {
    if (chunk.size() != chunk_size) {
      throw std::runtime_error("Got chunk of unexpected size from the network.");
    }

    if (error) {
      oak::log()->error("Unexpected error occurred reading from network.");

      std::unique_lock<std::mutex> lock{mutex_};
      condvar_.notify_all();
      return;
    }

    data_.append(chunk.begin(), chunk.end());
    if (data_.size() < content_length_) {
      read_chunk(conn);
    } else {
      std::unique_lock<std::mutex> lock{mutex_};
      condvar_.notify_all();
    }
  }
};

/**
 * Convert a cloe::Response to the ServerImpl::response type.
 */
void to_response_impl(const cloe::Response& r, ServerImpl::connection_ptr conn) {
  auto code = ServerImpl::connection::status_t(static_cast<int>(r.status()));
  auto type = cloe::as_cstr(r.type());
  ServerImpl::response_header headers[] = {
      {"Access-Control-Allow-Origin", "*"},  // Required by React.js
      {"Content-Type", type},
      {"Content-Length", std::to_string(r.body().size())},
      {"Connection", "close"},
  };

  conn->set_status(code);
  conn->set_headers(boost::make_iterator_range(headers, headers + 4));
  conn->write(r.body());
}

}  // anonymous namespace

/**
 * Implementation of cloe::Request interface.
 */
class Request : public cloe::Request {
 private:
  const ServerImpl::request& req_;
  ServerImpl::connection_ptr conn_;

  // The endpoint and query map is parsed from req_.destination.
  std::string endpoint_;
  std::map<std::string, std::string> queries_;

  // The body is read asynchronously.
  std::string body_;

  // The method is parsed from a string.
  RequestMethod method_;

 public:
  /**
   * Create a new Request from ServerImpl.
   *
   * - This should not panic.
   */
  explicit Request(const ServerImpl::request& req, const ServerImpl::connection_ptr& conn)
      : req_(req)
      , conn_(conn)
      , endpoint_(Muxer<Handler>::normalize(req.destination))
      , queries_(parse_queries(req.destination)) {
    from_string(req.method, method_);
    if (method_ == RequestMethod::POST) {
      body_ = DataReceiver<ServerImpl>(req, conn).read_all();
    }
  }

  RequestMethod method() const override { return method_; }
  ContentType type() const override { return ContentType::UNKNOWN; }
  const std::string& body() const override { return body_; }
  const std::string& uri() const override { return req_.destination; }
  const std::string& endpoint() const override { return endpoint_; }
  const std::map<std::string, std::string>& query_map() const override { return queries_; }
};

ServerImplHandler::ServerImplHandler() {
  muxer.set_default([this](const cloe::Request& q, cloe::Response& r) {
    ::oak::log()->debug("404 {}", q.endpoint());
    r.not_found(Json{
        {"error", "cannot find handler"},
        {"endpoints", Json(this->muxer.routes())},
    });
  });
}

void ServerImplHandler::operator()(ServerImpl::request const& request,
                                   ServerImpl::connection_ptr conn) {
  try {
    Request q(request, conn);
    ::oak::log()->debug("{} {}", as_cstr(q.method()), q.endpoint());
    cloe::Response r;
    muxer.get(q.endpoint()).first(q, r);
    to_response_impl(r, conn);
  } catch (const std::exception& e) {
    Response err;
    err.error(StatusCode::SERVER_ERROR, std::string(e.what()));
    to_response_impl(err, conn);
  } catch (...) {
    Response err;
    err.error(StatusCode::SERVER_ERROR, std::string("unknown error occurred"));
    to_response_impl(err, conn);
  }
}

void ServerImplHandler::log(const ServerImpl::string_type& msg) {
  // Don't show the error message if it's this bug from cppnetlib.
  if (msg == "Bad file descriptor") {
    return;
  }
  ::oak::log()->error("{}", msg);
}

void ServerImplHandler::add(const std::string& key, Handler h) { muxer.add(key, h); }

cloe::Json ServerImplHandler::endpoints_to_json(const std::vector<std::string>& endpoints) const {
  cloe::Json j;
  for (const auto& endpoint : endpoints) {
    const RequestStub q;
    Response r;
    try {
      muxer.get(endpoint).first(q, r);
    } catch (std::logic_error& e) {
      // Silently ignore endpoints that require an implementation of any of
      // the Request's methods.
      continue;
    }
    if (r.status() == cloe::StatusCode::OK && r.type() == cloe::ContentType::JSON) {
      j[endpoint] = cloe::Json(r.body());
    }
  }
  return j;
}

void Server::listen() {
  if (listening_) {
    throw std::runtime_error("already listening");
  }
  listening_ = true;

  options_.reset(new ServerImpl::options(handler_));
  options_->reuse_address(true);
  options_->thread_pool(std::make_shared<boost::network::utils::thread_pool>(listen_threads_));
  options_->address(listen_addr_);
  options_->port(std::to_string(listen_port_));
  server_.reset(new ServerImpl(*options_));
  thread_.reset(new boost::thread(boost::bind(&ServerImpl::run, server_.get())));
}

void Server::stop() {
  if (!listening_) {
    throw std::runtime_error("not listening");
  }

  server_->stop();
  thread_->interrupt();
  if (thread_->joinable()) {
    thread_->join();
  }
  thread_.reset(nullptr);
  server_.reset(nullptr);
  options_.reset(nullptr);
  listening_ = false;
}

}  // namespace oak
