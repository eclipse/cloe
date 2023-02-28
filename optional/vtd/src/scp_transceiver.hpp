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
 * \file scp_transceiver.hpp
 * \see  scp_transceiver.cpp
 *
 * This header file contains all important definitions for SCP communication
 * with VTD. In particular, it contains the SCP client.
 *
 * TODO(ben): >
 *    The API is not yet fully stable. Sending messages seems to make sense,
 *    but there should be a more elegant way to receive and poll messages.
 *    Maybe register callbacks for certain kinds of messages.
 *    Idea: If we can read the top-most xml tag, then we can allow externals
 *    to register callbacks for these. Or callbacks for certain paths.
 *    The problem is that we'd have to go through all paths, if that were
 *    allowed. This might be inefficient, but this depends on how many
 *    SCP messages we receive.
 */

#pragma once

#include <string>  // for string
#ifdef VTD_API_2_2_0
  #include <scpIcd.h>
#else
  #include <VtdToolkit/scpIcd.h>
#endif
#include <cloe/core.hpp>                     // for Json, Error
#include <cloe/utility/tcp_transceiver.hpp>  // for TcpTransceiver

#include "vtd_logger.hpp"  // for scp_logger

namespace vtd {

/**
 * ScpMessage is the interface which all SCP messages implement.
 * This allows a struct to "be" an SCP message. You can set the fields as you
 * like, and then to_scp returns the message that is sent across the wire.
 */
class ScpMessage {
 public:
  virtual ~ScpMessage() = default;
  virtual std::string to_scp() const = 0;
};

/**
 * ScpError may be thrown when an error is detected in the SCP protocol.
 * These may or not be recoverable, and include such origins such as magic
 * number and version mismatch.
 *
 * \see  cloe::utility::TcpReadError
 */
class ScpError : public cloe::Error {
 public:
  using Error::Error;
  virtual ~ScpError() noexcept = default;
};

/**
 * ScpTransceiver is an SCP transceiver over TCP.
 */
class ScpTransceiver : public cloe::utility::TcpTransceiver {
 public:
  using TcpTransceiver::TcpTransceiver;
  ~ScpTransceiver() = default;

  void send(const ScpMessage& msg) { write(msg.to_scp()); }
  void send(const std::string& msg) { write(msg.c_str()); }
  void send(const char* msg) { write(msg); }

  // unstable API:
  bool has() const {
    return this->tcp_available_data() >= static_cast<std::streamsize>(sizeof(SCP_MSG_HDR_t));
  }

  std::string receive();

  friend void to_json(cloe::Json j, const ScpTransceiver& t) {
    j = cloe::Json{
        {"connection_endpoint", t.tcp_endpoint()},
        {"connection_ok", t.tcp_is_ok()},
        {"num_errors", t.num_errors_},
        {"num_messages_sent", t.num_sent_},
        {"num_messages_received", t.num_received_},
    };
  }

 protected:
  void write(const std::string& msg) { write(msg.c_str()); }
  void write(const char* msg);

 private:
  uint64_t num_errors_{0};
  uint64_t num_sent_{0};
  uint64_t num_received_{0};
};

class ScpTransceiverFactory : public cloe::utility::TcpTransceiverFactory<ScpTransceiver> {
 public:
  using TcpTransceiverFactory::TcpTransceiverFactory;
  ~ScpTransceiverFactory() = default;

 protected:
  cloe::Logger factory_logger() const override { return scp_logger(); }
  const char* instance_name() const override { return "ScpTransceiver"; }
};

}  // namespace vtd
