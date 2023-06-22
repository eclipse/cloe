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
 * \file cloe/utility/tcp_transceiver.hpp
 */

#pragma once

#include <chrono>   // for duration<>
#include <memory>   // for unique_ptr<>
#include <string>   // for string, to_string
#include <thread>   // for this_thread, sleep_for
#include <utility>  // for move

#include <boost/asio.hpp>  // for iostream

#include <cloe/core.hpp>                            // for Error, Logger
#include <cloe/core/abort.hpp>                      // for AbortFlag, abort_checkpoint
#include <cloe/utility/tcp_transceiver_config.hpp>  // for TcpTransceiverConfiguration

namespace cloe {
namespace utility {

/**
 * TcpReadError may be thrown when there is an error during reading.
 * It should be used something like this:
 *
 *     tcp_stream_.read(&msg, msg_size)
 *     if (!tcp_stream_) {
 *       throw TcpReadError(tcp_stream_.error().message());
 *     }
 */
class TcpReadError : public Error {
 public:
  using Error::Error;
  virtual ~TcpReadError() noexcept = default;
};

/**
 * TcpTransceiver is a TCP transceiver, which contains common methods
 * for creating a connection, sending, receiving, and tearing it down
 * again.
 */
class TcpTransceiver {
 public:
  TcpTransceiver() = default;
  TcpTransceiver(const std::string& host, uint16_t port) { tcp_connect(host, port); }
  virtual ~TcpTransceiver() { TcpTransceiver::tcp_disconnect(); }  // NOLINT

  /**
   * Attempts to set up a TCP connection to this host and port.
   * - If it cannot connect it throws a `std::runtime_error`.
   */
  void tcp_connect(const std::string& host, uint16_t port) {
    tcp_stream_.clear();
    tcp_stream_.connect(host, std::to_string(port));
    if (!tcp_stream_) {
      throw std::ios_base::failure(tcp_stream_.error().message());
    }
    tcp_connected_ = true;
    tcp_host_ = host;
    tcp_port_ = port;
  }

  /**
   * Returns true if this object should be connected.
   *
   * - It does not take errors into account.
   */
  bool tcp_is_connected() const { return tcp_connected_; }

  /**
   * Returns true if the underlying TCP stream reports ok.
   *
   * This only makes sense if tcp_is_connected() returns true.
   */
  bool tcp_is_ok() const { return static_cast<bool>(tcp_stream_); }

  /**
   * Close the underlying stream and mark this object as disconnected.
   *
   * It is not necessary to call this before destruction.
   */
  void tcp_disconnect() {
    tcp_stream_.close();
    tcp_connected_ = false;
  }

  uint16_t tcp_port() const { return tcp_port_; }
  const std::string& tcp_host() const { return tcp_host_; }
  std::string tcp_endpoint() const { return fmt::format("tcp://{}:{}", tcp_host_, tcp_port_); }

 protected:
  /**
   * Returns the amount of available data in the buffer and on the socket.
   */
  std::streamsize tcp_available_data() const {
    return tcp_stream_.rdbuf()->in_avail() + tcp_stream_.rdbuf()->available();
  }

  template <typename M>
  void tcp_send(M* msg, size_t sz) {
    tcp_stream_.write(reinterpret_cast<const char*>(msg), sz);
    tcp_stream_.flush();
  }

 protected:
  boost::asio::ip::tcp::iostream tcp_stream_;
  bool tcp_connected_{false};
  std::string tcp_host_{};
  uint16_t tcp_port_{};
};

/**
 * TcpTransceiverFactory helps you create TcpTransceiver types
 * by retrying connection attempts a configurable number of times.
 *
 * There are two values that can currently be configured: retry attempts and
 * retry delay.
 *
 * Retry attempts (`retry_attempts`) is the number of attempts to retry after
 * connection failure.
 * The value 0 indicates no attempts and is effectively the same as not using
 * this factory.
 * Any negative value indicates an infinite number of connection attempts; this
 * is not recommended, but can be useful in certain circumstances.
 *
 * The retry delay (`retry_delay`) is fraction of time in seconds that should
 * be waited between connection attempts.
 * The value 0 indicates that no time is waited, and is not recommended, as
 * this can tie up your system.
 */
template <typename T>
class TcpTransceiverFactory {
 public:
  TcpTransceiverFactory() = default;
  TcpTransceiverFactory(int attempts, std::chrono::duration<float> delay) {
    config_.retry_attempts = attempts;
    config_.retry_delay = delay;
  }
  explicit TcpTransceiverFactory(const TcpTransceiverConfiguration& c) : config_(c) {}
  explicit TcpTransceiverFactory(TcpTransceiverConfiguration&& c) : config_(std::move(c)) {}
  virtual ~TcpTransceiverFactory() = default;

  int retry_attempts() const { return config_.retry_attempts; }
  void set_retry_attempts(int attempts) { config_.retry_attempts = attempts; }

  std::chrono::duration<float> retry_delay() const { return config_.retry_delay; }
  void set_retry_delay(std::chrono::duration<float> delay) { config_.retry_delay = delay; }

  /**
   * Create a TcpTransceiver derived type or return nullptr.
   */
  std::unique_ptr<T> create_or_null(const std::string& host, uint16_t port) const {
    try {
      return this->create_or_throw(host, port);
    } catch (std::ios_base::failure&) {
      return std::unique_ptr<T>{nullptr};
    }
  }

  /**
   * Create an RdbTranscieverTcp or throw a `std::ios_base::failure`.
   */
  std::unique_ptr<T> create_or_throw(const std::string& host, uint16_t port) const {
    // On a 32-bit machine, this will overflow in 48 days.
    for (int32_t attempts = 0; true; attempts++) {
      try {
        if (attempts == 0) {
          factory_logger()->info("{} connect tcp://{}:{}", instance_name(), host, port);
        } else {
          factory_logger()->info("{} connect tcp://{}:{} [attempt {}/{}]", instance_name(), host,
                                 port, attempts + 1, config_.retry_attempts + 1);
        }
        return std::make_unique<T>(host, port);
      } catch (std::ios_base::failure&) {
        if (attempts == config_.retry_attempts) {
          throw;
        }
      }
      std::this_thread::sleep_for(config_.retry_delay);
    }
  }

  /**
   * Create an RdbTranscieverTcp or throw a `std::ios_base::failure`.
   *
   * If an abort is signalled, an `AsyncAbort` is thrown.
   */
  std::unique_ptr<T> create_or_throw(const std::string& host, uint16_t port, AbortFlag& sig) const {
    // On a 32-bit machine, this will overflow in 48 days.
    for (int32_t attempts = 0; true; attempts++) {
      try {
        if (attempts == 0) {
          factory_logger()->info("{} connect tcp://{}:{}", instance_name(), host, port);
        } else {
          factory_logger()->info("{} connect tcp://{}:{} [attempt {}/{}]", instance_name(), host,
                                 port, attempts + 1, config_.retry_attempts + 1);
        }
        return std::make_unique<T>(host, port);
      } catch (std::ios_base::failure&) {
        if (attempts == config_.retry_attempts) {
          throw;
        }
      }
      abort_checkpoint(sig);
      std::this_thread::sleep_for(config_.retry_delay);
    }
  }

  friend void to_json(Json& j, const TcpTransceiverFactory<T>& f) { to_json(j, f.config_); }
  friend void from_json(const Json& j, TcpTransceiverFactory<T>& f) { from_json(j, f.config_); }

 protected:
  virtual Logger factory_logger() const = 0;
  virtual const char* instance_name() const = 0;

 protected:
  TcpTransceiverConfiguration config_;
};

/**
 * Return a new TcpTransceiver of the given type or throws an exception.
 *
 * This is a wrapper around creating a one-shot TcpTransceiverFactory.
 */
template <typename F>
auto create_or_throw_with(const TcpTransceiverFullConfiguration& c)
    -> decltype(F{c}.create_or_throw(c.host, c.port)) {
  return F{c}.create_or_throw(c.host, c.port);
}

/**
 * Return a new TcpTransceiver of the given type or throws an exception.
 *
 * This is a wrapper around creating a one-shot TcpTransceiverFactory.
 */
template <typename F>
auto create_or_throw_with(const TcpTransceiverFullConfiguration& c, AbortFlag& sig)
    -> decltype(F{c}.create_or_throw(c.host, c.port, sig)) {
  return F{c}.create_or_throw(c.host, c.port, sig);
}

/**
 * Return a new TcpTransceiver of the given type or returns nullptr.
 *
 * This is a wrapper around creating a one-shot TcpTransceiverFactory.
 */
template <typename F>
auto create_or_null_with(const TcpTransceiverFullConfiguration& c)
    -> decltype(F{c}.create_or_null(c.host, c.port)) {
  return F{c}.create_or_null(c.host, c.port);
}

}  // namespace utility
}  // namespace cloe
