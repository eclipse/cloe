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
 * \file rdb_transceiver_tcp.cpp
 */

#include "rdb_transceiver_tcp.hpp"

#include <cassert>  // for assert
#include <cstdlib>  // for malloc, free
#include <memory>   // for shared_ptr<>

#include <cloe/utility/tcp_transceiver.hpp>  // for TcpReadError

namespace vtd {

// This method reads a complete RDB_MSG_t from the stream.
//
// First, we read the header of the message to find out how much memory we have to allocate,
// and then we read the rest of the data after verifying the validity of the header.
std::shared_ptr<RDB_MSG_t> RdbTransceiverTcp::receive_wait() {
  // 1.a) Allocate enough memory for the header
  auto msg_hdr = reinterpret_cast<RDB_MSG_HDR_t*>(malloc(sizeof(RDB_MSG_HDR_t)));
  if (msg_hdr == nullptr) {
    // This does not count as an error, because it's not connection related.
    throw cloe::Error("RdbTransceiverTcp: malloc failed");
  }

  // 1.b) Read the header
  tcp_stream_.read(reinterpret_cast<char*>(msg_hdr), sizeof(RDB_MSG_HDR_t));
  if (!tcp_stream_) {
    free(msg_hdr);
    this->num_errors_++;
    throw cloe::utility::TcpReadError("RdbTransceiverTcp: error during read: {}",
                                      tcp_stream_.error().message());
  }

  // 2. Verify if header is in sync
  if (RDB_MAGIC_NO != msg_hdr->magicNo) {
    free(msg_hdr);
    this->num_errors_++;
    throw RdbError("RdbTransceiverTcp: magic number does not match");
  }
  assert(sizeof(*msg_hdr) == msg_hdr->headerSize);

  // 3. Allocate full amount of memory and append data
  auto msg =
      reinterpret_cast<RDB_MSG_t*>(realloc(msg_hdr, msg_hdr->headerSize + msg_hdr->dataSize));
  tcp_stream_.read(reinterpret_cast<char*>(&(msg->entryHdr)), msg->hdr.dataSize);
  if (!tcp_stream_) {
    free(msg);
    this->num_errors_++;
    throw cloe::utility::TcpReadError("RdbTransceiverTcp: error during read: {}",
                                      tcp_stream_.error().message());
  }

  // 4. Wrap the result in a shared_ptr and return it
  return std::shared_ptr<RDB_MSG_t>(msg);
}

}  // namespace vtd
