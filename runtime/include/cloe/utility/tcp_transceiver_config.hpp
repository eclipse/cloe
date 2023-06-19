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
 * \file cloe/utility/tcp_transceiver_config.hpp
 * \see  cloe/utility/tcp_transceiver.hpp
 */

#pragma once

#include <chrono>   // for duration
#include <string>   // for string
#include <utility>  // for move

#include <cloe/core.hpp>  // for Confable, Schema, Json

namespace cloe {
namespace utility {

/**
 * This configuration struct is meant to be re-used in various other
 * configuration blocks for connection configuration.
 */
struct TcpTransceiverConfiguration : public Confable {
  TcpTransceiverConfiguration() = default;
  TcpTransceiverConfiguration(int32_t attempts, std::chrono::duration<float> delay)
      : retry_attempts(attempts), retry_delay(delay) {}
  virtual ~TcpTransceiverConfiguration() = default;

  /**
   * Retry attempts is the number of attempts to retry after connection failure.
   * The value 0 indicates no attempts and is effectively the same as not using
   * this factory.
   * Any negative value indicates an infinite number of connection attempts;
   * this is not recommended, but can be useful in certain circumstances.
   */
  int32_t retry_attempts{60};

  /**
   * The retry delay is the fraction of time in seconds that should
   * be waited between connection attempts.
   * The value 0 indicates that no time is waited, and is not recommended, as
   * this can tie up your system.
   */
  std::chrono::duration<float> retry_delay{1.0};

  CONFABLE_SCHEMA(TcpTransceiverConfiguration) {
    return Schema{
        {"retry_attempts", Schema(&retry_attempts, "connection retry attempts")},
        {"retry_delay_s", Schema(&retry_delay, "time delay between connection attempts")},
    };
  }

  void to_json(Json& j) const override {
    j = {
        {"retry_attempts", retry_attempts},
        {"retry_delay_s", retry_delay.count()},
    };
  }
};

/**
 * This configuration struct is meant to be re-used in various other
 * configuration blocks for connection configuration.
 *
 * Usually, the host and port values will be set to some default.
 */
struct TcpTransceiverFullConfiguration : public TcpTransceiverConfiguration {
  TcpTransceiverFullConfiguration() = default;
  TcpTransceiverFullConfiguration(std::string host, uint16_t port)
      : host(std::move(host)), port(port) {}

  /**
   * Hostname or IP address for the TCP connection.
   */
  std::string host{"localhost"};

  /**
   * Port for TCP connection.
   */
  uint16_t port{0};

  CONFABLE_SCHEMA(TcpTransceiverFullConfiguration) {
    return Schema{
        TcpTransceiverConfiguration::schema_impl(),
        {
            {"host", Schema(&host, "hostname of connection")},
            {"port", Schema(&port, "port of connection")},
        },
    };
  }

  void to_json(Json& j) const override {
    TcpTransceiverConfiguration::to_json(j);
    j["host"] = host;
    j["port"] = port;
  }
};

}  // namespace utility
}  // namespace cloe
