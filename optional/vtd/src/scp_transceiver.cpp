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
 * \file scp_transceiver.cpp
 */

#include "scp_transceiver.hpp"

#include <iostream>  // for snprintf
#include <string>    // for string

#include <cloe/utility/tcp_transceiver.hpp>  // for TcpReadError

#include "vtd_version.hpp"
#if (VTD_API_VERSION_EPOCH == 0)
  #include <scpIcd.h> // for SCP_MAGIC_NO, SCP_VERSION, SCP_MSG_HDR_t, ...
#else
  #include <VtdToolkit/scpIcd.h>
#endif

namespace vtd {

std::string ScpTransceiver::receive() {
  SCP_MSG_HDR_t msg_hdr;

  // read header
  tcp_stream_.read(reinterpret_cast<char*>(&msg_hdr), sizeof(SCP_MSG_HDR_t));
  if (!tcp_stream_) {
    num_errors_++;
    throw cloe::utility::TcpReadError("ScpTransceiver: error during read: {}",
                                      tcp_stream_.error().message());
  }

  // check if header is in sync
  if (SCP_MAGIC_NO != msg_hdr.magicNo) {
    num_errors_++;
    throw ScpError("ScpTransceiver: magic number does not match");
  }

  // check if version number matches
  if (SCP_VERSION != msg_hdr.version) {
    num_errors_++;
    throw ScpError("ScpTransceiver: version number does not match");
  }

  // allocate memory and append data
  std::string msg;
  msg.resize(msg_hdr.dataSize);
  tcp_stream_.read(&(msg[0]), msg_hdr.dataSize);
  if (!tcp_stream_) {
    num_errors_++;
    throw cloe::utility::TcpReadError("ScpTransceiver: error during read: {}",
                                      tcp_stream_.error().message());
  }
  num_received_++;
  scp_logger()->trace("ScpTransceiver: received {}", msg);
  return msg;
}

void ScpTransceiver::write(const char* msg) {
  SCP_MSG_HDR_t msg_hdr;
  msg_hdr.magicNo = SCP_MAGIC_NO;
  msg_hdr.version = SCP_VERSION;
  // TODO(ben): Make sender and receiver name configurable.
  snprintf(msg_hdr.sender, SCP_NAME_LENGTH, "cloe");
  snprintf(msg_hdr.receiver, SCP_NAME_LENGTH, "any");
  msg_hdr.dataSize = static_cast<uint32_t>(strlen(msg));
  //                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  //                 Ok from size_t to uint32_t, our messages are expected to
  //                 be shorter than 4GB.

  tcp_stream_.write(reinterpret_cast<char*>(&msg_hdr), sizeof(msg_hdr));
  tcp_stream_.write(msg, msg_hdr.dataSize);
  tcp_stream_.flush();
  num_sent_++;
  scp_logger()->trace("ScpTransceiver: sent {}", msg);
}

}  // namespace vtd
