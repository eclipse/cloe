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

#include <chrono>
#include <condition_variable>  // for condition_variable
#include <ctime>
#include <functional>  // for bind
#include <iostream>
#include <map>      // for map<>
#include <memory>   // for make_shared<>
#include <mutex>    // for mutex, unique_lock
#include <sstream>  // for stringstream
#include <string>   // for string
#include <thread>   // for thread

#include <boost/beast/core.hpp>  // for boost::beast
#include <boost/beast/http.hpp>  // for boost::beast::http
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>  // for tokenizer, char_separator
#include <boost/version.hpp>    // for BOOST_VERSION

#include <cloe/core.hpp>        // for logger::get
#include <cloe/handler.hpp>     // for Request
#include "oak/requeststub.hpp"  // for RequestStub
using namespace cloe;           // NOLINT(build/namespaces)

// Alias namespaces
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace beast = boost::beast;
namespace http = boost::beast::http;

namespace oak {

// The type of the used socket
using socket_type = ip::tcp::socket;
// The type of the used flat buffer
using flat_buffer_type = beast::flat_buffer;
// The type of the request message
using request_type = http::request<http::string_body>;
// The type of the response message
using response_type = http::response<http::dynamic_body>;

// Forward declarations
class http_connection;

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

}  // anonymous namespace

/**
 * Implementation of cloe::Request interface.
 */
class Request : public cloe::Request {
 private:
  const http_connection& connection_;
  const request_type& request_;
  const std::string uri_;

  // The endpoint and query map is parsed from req_.destination.
  std::string endpoint_;
  std::map<std::string, std::string> queries_;

  // The body is read asynchronously.
  std::string body_;

  // The method is parsed from a string.
  RequestMethod method_;

 public:
  /**
   * Creates a new Request from a http_connection
   */
  explicit Request(const http_connection& connection);

  RequestMethod method() const override { return method_; }
  ContentType type() const override { return ContentType::UNKNOWN; }
  const std::string& body() const override { return body_; }
  const std::string& uri() const override { return uri_; }
  const std::string& endpoint() const override { return endpoint_; }
  const std::map<std::string, std::string>& query_map() const override { return queries_; }
};

/**
 * @brief Represents the http-connection
 */
class http_connection : public std::enable_shared_from_this<http_connection> {
 public:
  http_connection(ip::tcp::socket socket, Muxer<Handler>& muxer)
      : socket_(std::move(socket))
#if (BOOST_VERSION / 100000 == 1)
#if (BOOST_VERSION / 100 % 1000 > 69)
      , deadline_(socket_.get_executor())
#else
      , deadline_(socket.get_io_service())
#endif
#else
#error unexpected boost version
#endif
      , muxer_(muxer) {
    deadline_.expires_after(std::chrono::seconds(60));
  }

  void start() {
    read_request();
    check_deadline();
  }

  /**
   * @brief Returns the boost::beast request
   *
   * @return const request_type& The boost::beast request
   */
  inline const request_type& getRequest() const { return request_; }

 private:
  /**
   * @brief The socket for the currently connected client.
   */
  socket_type socket_;
  /**
   * @brief The buffer for performing reads.
   */
  flat_buffer_type buffer_{8192};
  /**
   * @brief The request message.
   */
  request_type request_;
  /**
   * @brief The response message.
   */
  response_type response_;
  /**
   * @brief The timer for timeout mechanism
   */
  asio::steady_timer deadline_;
  /**
   * @brief Cloe::muxer
   */
  Muxer<Handler>& muxer_;

  /**
   * @brief Asynchronously receives a complete request message.
   */
  void read_request() {
    // Keep reference to this
    auto self = shared_from_this();
    // Commence async read
    http::async_read(socket_, buffer_, request_,
                     [self](beast::error_code ec, std::size_t bytes_transferred) {
                       boost::ignore_unused(bytes_transferred);
                       if (!ec) self->process_request();
                     });
  }

  /**
   * @brief Asynchronously transmits the response message.
   */
  void write_response() {
    // Keep reference to this
    auto self = shared_from_this();
    // Set correct content length
    response_.content_length(response_.body().size());
    // Commence async write
    http::async_write(socket_, response_, [self](beast::error_code ec, std::size_t) {
      self->socket_.shutdown(ip::tcp::socket::shutdown_send, ec);
      self->deadline_.cancel();
    });
  }

  /**
   * @brief Implements a timeout mechanism
   */
  void check_deadline() {
    // Keep reference to this
    auto self = shared_from_this();
    deadline_.async_wait([self](beast::error_code ec) {
      if (!ec) {
        // Close socket to cancel any outstanding operation.
        self->socket_.close(ec);
      }
    });
  }

  /**
   * @brief Processes the request
   */
  void process_request() {
    try {
      Request q(*this);
      ::oak::log()->debug("{} {}", as_cstr(q.method()), q.endpoint());
      cloe::Response r;
      muxer_.get(q.endpoint()).first(q, r);
      to_response_impl(r);
    } catch (const std::exception& e) {
      Response err;
      err.error(StatusCode::SERVER_ERROR, std::string(e.what()));
      to_response_impl(err);
    } catch (...) {
      Response err;
      err.error(StatusCode::SERVER_ERROR, std::string("unknown error occurred"));
      to_response_impl(err);
    }
    write_response();
  }
  /**
   * Convert a cloe::Response to response_type.
   */
  void to_response_impl(const cloe::Response& r) {
    response_.result(static_cast<http::status>(
        static_cast<std::underlying_type_t<cloe::StatusCode>>(r.status())));
    response_.set(http::field::server, "cloe");
    response_.set(http::field::access_control_allow_origin,
                  "*");  // Required by React.js
    response_.set(http::field::content_type, cloe::as_cstr(r.type()));
    response_.set(http::field::content_length, std::to_string(r.body().size()));
    response_.set(http::field::connection, "close");
    http::dynamic_body::value_type body;
    const auto body_length = r.body().length();
    const auto body_data = static_cast<const void*>(r.body().c_str());
    size_t n = asio::buffer_copy(body.prepare(body_length), asio::buffer(body_data, body_length));
    body.commit(n);
    response_.body() = std::move(body);
  }
};

Request::Request(const http_connection& connection)
    : connection_(connection)
    , request_(connection.getRequest())
    , uri_(request_.target())
    , endpoint_(Muxer<Handler>::normalize(uri_))
    , queries_(parse_queries(uri_)) {
  // Convert boost request to Cloe::Request
  switch (request_.method()) {
    case http::verb::get:
      method_ = RequestMethod::GET;
      break;
    case http::verb::post:
      method_ = RequestMethod::POST;
      break;
    case http::verb::put:
      method_ = RequestMethod::PUT;
      break;
    case http::verb::delete_:
      method_ = RequestMethod::DELETE;
      break;
    default:
      throw new std::runtime_error("unexpected http request-method");
      break;
  }
  if (method_ == RequestMethod::POST) {
    body_ = request_.body();
  }
}

void Server::init() {
  muxer_.set_default([this](const cloe::Request& q, cloe::Response& r) {
    ::oak::log()->debug("404 {}", q.endpoint());
    r.not_found(Json{
        {"error", "cannot find handler"},
        {"endpoints", Json(this->muxer_.routes())},
    });
  });
}

cloe::Json Server::endpoints_to_json(const std::vector<std::string>& endpoints) const {
  cloe::Json j;
  for (const auto& endpoint : endpoints) {
    const RequestStub q;
    Response r;
    try {
      muxer_.get(endpoint).first(q, r);
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
  if (state_.load() != ServerState::Default) {
    throw std::runtime_error("already listening");
  }

  state_ = ServerState::Init;
  try {
    // Bind address and acceptor mechanism
    auto const address = ip::make_address(listen_addr_);
    unsigned short port = static_cast<unsigned short>(listen_port_);

    acceptor_ = ip::tcp::acceptor(ioc_, {address, port});
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    socket_ = ip::tcp::socket(ioc_);
    serve(acceptor_, socket_);

  } catch (boost::wrapexcept<boost::system::system_error>& /*ex*/) {
    state_ = ServerState::Default;
    return;
  } catch (...) {
    state_ = ServerState::Default;
    return;
  }
  thread_ = std::move(std::thread([this]() { this->server_thread(); }));
  while (state_.load() == ServerState::Init) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds{1});
  }
}

void Server::serve(ip::tcp::acceptor& acceptor, ip::tcp::socket& socket) {
  acceptor.async_accept(socket, [&](beast::error_code ec) {
    if (!ec) std::make_shared<http_connection>(std::move(socket), muxer_)->start();
    serve(acceptor, socket);
  });
}

void Server::server_thread() {
  // Create additional worker threads and pump messages on those and this one
  auto worker_threads = std::max(listen_threads_, (unsigned int)1) - 1;
  std::vector<std::thread> v;
  v.reserve(worker_threads);
  for (size_t i = 0; i < worker_threads; ++i) {
    v.emplace_back([this] { this->server_thread_impl(); });
  }
  server_thread_impl();

  // join all workers threads
  for (auto& thread : v) {
    thread.join();
  }

  // Server is stopped
  state_ = ServerState::Stopped;
}

void Server::server_thread_impl() {
  state_ = ServerState::Listening;
  ioc_.run();  // discard return value
  state_ = ServerState::Stopping;
}

void Server::stop() {
  if (state_.load() == ServerState::Default) {
    throw std::runtime_error("not listening");
  }
  // stop the IOContext
  ioc_.stop();
  // join our worker-thread(s)
  if (thread_.joinable()) {
    thread_.join();
  }
  state_ = ServerState::Default;
}

}  // namespace oak
