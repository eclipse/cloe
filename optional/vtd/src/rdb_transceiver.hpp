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
 * \file rdb_transceiver.hpp
 * \see  rdb_transceiver_tcp.hpp
 * \see  rdb_transceiver_shm.hpp
 */

#pragma once

#include <memory>  // for shared_ptr<>
#include <vector>  // for vector<>
#ifdef VTD_API_2_2_0
  #include <viRDBIcd.h>  // for RDB_MSG_t
#else
  #include <VtdToolkit/viRDBIcd.h>
#endif
#include <cloe/core.hpp>  // for Json, Error

namespace vtd {

/**
 * RdbError may be thrown when an error is detected in the RDB protocol.
 * These may or not be recoverable, and include such origins such as magic
 * number and version mismatch.
 *
 * \see  cloe::utility::TcpReadError
 */
class RdbError : public cloe::Error {
 public:
  using Error::Error;
  virtual ~RdbError() noexcept = default;
};

/**
 * RdbTransceiver is an interface for a RDB connection to VTD.
 *
 * There are currently two implementations of this: RDB over TCP and over shared memory.
 * Currently, the shared memory implementation is not being used.
 */
class RdbTransceiver {
 public:
  virtual ~RdbTransceiver() = default;

  /**
   * Returns true when the transceiver has a message that can be received.
   *
   * That is, if true, then a call to receive() will return a vector
   * that is not empty.
   */
  virtual bool has() const = 0;

  /**
    * Non-blocking function to return all received RDB messages.
    */
  virtual std::vector<std::shared_ptr<RDB_MSG_t>> receive() = 0;

  /**
   * Sends the RDB message with the given size.
   *
   * \param msg RDB message
   * \param size number of bytes
   */
  virtual void send(const RDB_MSG_t* msg, size_t size) = 0;

  virtual void to_json(cloe::Json& j) const = 0;

  friend void to_json(cloe::Json& j, const RdbTransceiver& t) { t.to_json(j); }
};

}  // namespace vtd
