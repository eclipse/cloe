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
 * \file rdb_transceiver_shm.hpp
 * \see  rdb_transceiver_shm.cpp
 */

#pragma once

#include <memory>  // for unique_ptr<>
#include <vector>  // for vector<>

#include <boost/interprocess/mapped_region.hpp>  // for mapped_region

#include <cloe/core.hpp>  // for Json

#include "rdb_transceiver.hpp"  // for RdbTransceiver

namespace vtd {

/**
 * This class implements an RDB client via shared memory.
 *
 * WARNING:
 * > This class is primarily a proof-of-concept. Currently, it is not
 * > used and there is no guarantee that it actually works as advertised.
 * >
 * > Even if it works, under the hood manual memory-management is used,
 * > which means that until it is audited, it may leak memory or worse
 * > (ironic, I know).
 */
class RdbTransceiverShm : public RdbTransceiver {
 public:
  /**
   * Connect to VTD memory to create a new RDB communication.
   *
   * If a "connection" cannot be achieved, C-strings are thrown.
   *
   * \param key to obtain shared memory id
   * \param release_mask mask used by VTD to mask a shared memory region as accessible
   */
  RdbTransceiverShm(key_t key, uint32_t release_mask);

  virtual ~RdbTransceiverShm();

  bool has() const override {
    throw std::runtime_error("RdbTransceiverShm: has not implmented yet");
  }

  std::vector<std::shared_ptr<RDB_MSG_t>> receive() override;

  void send(const RDB_MSG_t*, size_t) override {
    throw std::runtime_error("RdbTransceiverShm: send not implemented");
  }

  void to_json(cloe::Json& j) const override {
    j = cloe::Json{
        {"connection_endpoint", "shm://unknown-key"},
        {"num_errors", this->num_errors_},
        {"num_messages", this->num_messages_},
    };
  }

  friend void to_json(cloe::Json& j, const RdbTransceiverShm& t) { t.to_json(j); }

 protected:
  /// VTD uses this mask to notify client when data in buffer is ready.
  uint32_t release_mask_;

  /// Shared memory region.
  boost::interprocess::mapped_region region_;

  /// Pointer to the shared memory management header.
  RDB_SHM_HDR_t* rdb_shm_hdr_{nullptr};

  /// Array of pointers to buffer information.
  RDB_SHM_BUFFER_INFO_t** buffer_info_{nullptr};

  /// Array of pointers to rdb messages.
  RDB_MSG_t** rdb_msg_{nullptr};

  // Hold on to some cheap statistics for the JSON representation.
  uint64_t num_errors_{0};
  uint64_t num_messages_{0};
};

}  // namespace vtd
