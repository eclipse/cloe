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
 * \file rdb_transceiver_shm.cpp
 * \see  rdb_transceiver_shm.hpp
 */

#include "rdb_transceiver_shm.hpp"

#include <sys/shm.h>  // for shmget
#include <cstdlib>    // for memcopy, malloc
#include <memory>     // for shared_ptr<>
#include <stdexcept>  // for runtime_error
#include <vector>     // for vector<>

#include <boost/interprocess/xsi_shared_memory.hpp>  // for xsi_shared_memory, ...

#include <RDBHandler.hh>  // for RDB_SHM_BUFFER_INFO_t, RDB_MSG_t, ...

#include "vtd_logger.hpp"  // for rdb_logger

namespace vtd {

RdbTransceiverShm::RdbTransceiverShm(key_t key, uint32_t release_mask)
    : RdbTransceiver(), release_mask_(release_mask) {
  int shm_id = shmget(key, 0, 0);

  if (-1 == shm_id) {
    throw std::runtime_error("RdbTransceiverShm: failed to get shared memory ID");
  }

  // Initialize shared
  boost::interprocess::xsi_shared_memory shm(boost::interprocess::open_only, shm_id);
  region_ = boost::interprocess::mapped_region(shm, boost::interprocess::read_write);
  rdb_shm_hdr_ = reinterpret_cast<RDB_SHM_HDR_t *>(region_.get_address());
  if (2 != rdb_shm_hdr_->noBuffers) {
    throw std::runtime_error("RdbTransceiverShm: double buffering required");
  }

  // Store pointers to RDB_SHM_BUFFER_INFO_t and pointer to RDB_MSG_t for each
  // SHM buffer respectively
  buffer_info_ = new RDB_SHM_BUFFER_INFO_t *[rdb_shm_hdr_->noBuffers];
  rdb_msg_ = new RDB_MSG_t *[rdb_shm_hdr_->noBuffers];

  if (0 == rdb_shm_hdr_->dataSize) {
    throw std::runtime_error("RdbTransceiverShm: rdb_shm_hdr->dataSize is zero");
  }

  // Store pointers to RDB_SHM_BUFFER_INFO_t
  buffer_info_[0] = reinterpret_cast<RDB_SHM_BUFFER_INFO_t *>(
      reinterpret_cast<char *>(rdb_shm_hdr_) + rdb_shm_hdr_->headerSize);
  for (int i = 1; i < rdb_shm_hdr_->noBuffers; ++i) {
    buffer_info_[i] = reinterpret_cast<RDB_SHM_BUFFER_INFO_t *>(
        reinterpret_cast<char *>(buffer_info_[i - 1]) + buffer_info_[i - 1]->thisSize);
  }

  // Store pointers to first RDB_MSG_t in each RDB_SHM_BUFFER_INFO_t
  for (int i = 0; i < rdb_shm_hdr_->noBuffers; ++i) {
    rdb_msg_[i] = reinterpret_cast<RDB_MSG_t *>(reinterpret_cast<char *>(rdb_shm_hdr_) +
                                                buffer_info_[i]->offset);
    buffer_info_[i]->flags = 0;
  }
}

RdbTransceiverShm::~RdbTransceiverShm() {
  delete[] buffer_info_;
  delete[] rdb_msg_;
}

std::vector<std::shared_ptr<RDB_MSG_t>> RdbTransceiverShm::receive() {
  int buffer_id = -1;

  std::vector<std::shared_ptr<RDB_MSG_t>> messages;

  if (0 == rdb_shm_hdr_->dataSize) {
    return messages;
  }

  // Store pointers to RDB_SHM_BUFFER_INFO_t
  buffer_info_[0] = reinterpret_cast<RDB_SHM_BUFFER_INFO_t *>(
      reinterpret_cast<char *>(rdb_shm_hdr_) + rdb_shm_hdr_->headerSize);
  for (int i = 1; i < rdb_shm_hdr_->noBuffers; ++i) {
    buffer_info_[i] = reinterpret_cast<RDB_SHM_BUFFER_INFO_t *>(
        reinterpret_cast<char *>(buffer_info_[i - 1]) + buffer_info_[i - 1]->thisSize);
  }

  // Store pointers to first RDB_MSG_t in each RDB_SHM_BUFFER_INFO_t
  for (int i = 0; i < rdb_shm_hdr_->noBuffers; ++i) {
    rdb_msg_[i] = reinterpret_cast<RDB_MSG_t *>(reinterpret_cast<char *>(rdb_shm_hdr_) +
                                                buffer_info_[i]->offset);
  }

  for (int i = 0; i < rdb_shm_hdr_->noBuffers; ++i) {
    if (RDB_MAGIC_NO != rdb_msg_[i]->hdr.magicNo) {
      rdb_logger()->error("RdbTransceiverShm: magic number does not match: {}",
                          rdb_msg_[i]->hdr.magicNo);
      this->num_errors_++;
      return messages;
    }
  }
  bool is_ready[2] = {((buffer_info_[0]->flags & release_mask_) || !release_mask_) &&
                          !(buffer_info_[0]->flags & RDB_SHM_BUFFER_FLAG_LOCK),
                      ((buffer_info_[1]->flags & release_mask_) || !release_mask_) &&
                          !(buffer_info_[1]->flags & RDB_SHM_BUFFER_FLAG_LOCK)};

  if (is_ready[0] && is_ready[1]) {
    if (rdb_msg_[0]->hdr.frameNo < rdb_msg_[1]->hdr.frameNo) {
      buffer_id = 0;
    } else {
      buffer_id = 1;
    }
  } else if (is_ready[0]) {
    buffer_id = 0;
  } else if (is_ready[1]) {
    buffer_id = 1;
  }

  if (is_ready[0] || is_ready[1]) {
    buffer_info_[buffer_id]->flags |= RDB_SHM_BUFFER_FLAG_LOCK;

    // copy messages from SHM to local memory
    while (RDB_MAGIC_NO == rdb_msg_[buffer_id]->hdr.magicNo) {
      RDB_MSG_t *message = reinterpret_cast<RDB_MSG_t *>(
          malloc(rdb_msg_[buffer_id]->hdr.headerSize + rdb_msg_[buffer_id]->hdr.dataSize));
      memcpy(message, rdb_msg_[buffer_id],
             rdb_msg_[buffer_id]->hdr.headerSize + rdb_msg_[buffer_id]->hdr.dataSize);
      this->num_messages_++;
      messages.push_back(std::shared_ptr<RDB_MSG_t>{message});
      rdb_msg_[buffer_id] = reinterpret_cast<RDB_MSG_t *>(
          reinterpret_cast<char *>(rdb_msg_[buffer_id]) + rdb_msg_[buffer_id]->hdr.headerSize +
          rdb_msg_[buffer_id]->hdr.dataSize);
    }

    buffer_info_[buffer_id]->flags &= ~release_mask_;
    buffer_info_[buffer_id]->flags &= ~RDB_SHM_BUFFER_FLAG_LOCK;
  }

  return messages;
}

}  // namespace vtd
