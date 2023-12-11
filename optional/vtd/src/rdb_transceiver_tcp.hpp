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
 * \file rdb_transceiver_tcp.hpp
 * \see  rdb_transceiver_shm.hpp
 * \see  rdb_transceiver_tcp.cpp
 */

#pragma once

#include <memory>  // for shared_ptr<>
#include <vector>  // for vector<>

#include <boost/asio.hpp>  // for streamsize

#include <cloe/core.hpp>                     // for Json
#include <cloe/utility/tcp_transceiver.hpp>  // for TcpTransceiver

#include "rdb_transceiver.hpp"  // for RdbTransceiver
#include "vtd_logger.hpp"       // for rdb_logger

#define VTD_RDB_WAIT_SLEEP_MS 1

namespace vtd {

/**
 * RdbTransceiverTcp implements an RdbTransceiver via TCP.
 */
class RdbTransceiverTcp : public RdbTransceiver, public cloe::utility::TcpTransceiver {
 public:
  using TcpTransceiver::TcpTransceiver;

  bool has() const override {
    return this->tcp_available_data() >= static_cast<std::streamsize>(sizeof(RDB_MSG_HDR_t));
  }

  std::vector<std::shared_ptr<RDB_MSG_t>> receive() override {
    std::vector<std::shared_ptr<RDB_MSG_t>> msgs;
    while (!this->has()) {
      std::this_thread::sleep_for(cloe::Milliseconds{VTD_RDB_WAIT_SLEEP_MS});
    }
    while (this->has()) {
      num_received_++;
      msgs.push_back(this->receive_wait());
    }
    return msgs;
  }

  void send(const RDB_MSG_t* message, size_t size) override {
    num_sent_++;
    this->tcp_send(message, size);
  }

  void to_json(cloe::Json& j) const override {
    j = cloe::Json{
        {"connection_endpoint", this->tcp_endpoint()},
        {"connection_ok", this->tcp_is_ok()},
        {"num_errors", this->num_errors_},
        {"num_messages_sent", this->num_sent_},
        {"num_messages_received", this->num_received_},
    };
  }

  friend void to_json(cloe::Json& j, const RdbTransceiverTcp& t) { t.to_json(j); }

 protected:
  /**
   * Synchronous (blocking) method to receive an RDB message.
   */
  std::shared_ptr<RDB_MSG_t> receive_wait();

 private:
  // Statistics for interest's sake
  uint64_t num_errors_{0};
  uint64_t num_sent_{0};
  uint64_t num_received_{0};
};

class RdbTransceiverTcpFactory : public cloe::utility::TcpTransceiverFactory<RdbTransceiverTcp> {
 public:
  using TcpTransceiverFactory::TcpTransceiverFactory;

 protected:
  cloe::Logger factory_logger() const override { return rdb_logger(); }
  const char* instance_name() const override { return "RdbTransceiverTcp"; }
};

}  // namespace vtd
