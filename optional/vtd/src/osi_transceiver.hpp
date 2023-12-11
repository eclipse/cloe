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
 * \file osi_transceiver.hpp
 * \see  osi_transceiver_tcp.hpp
 */

#pragma once

#include <cloe/core.hpp>  // for Json, Error

#include <osi3/osi_sensordata.pb.h>  // for SensorData

namespace osii {

/**
 * OsiError may be thrown when an error is detected in the OSI protocol.
 * These may or not be recoverable, and include such origins such as data
 * format and version mismatch.
 *
 * \see cloe::utility::TcpReadError
 */
class OsiError : public cloe::Error {
 public:
  using Error::Error;
  virtual ~OsiError() noexcept = default;
};

/**
 * OsiTransceiver is an interface for a OSI connection via TCP.
 */
class OsiTransceiver {
 public:
  virtual ~OsiTransceiver() = default;

  /**
   * Return true when the transceiver has a SensorData message that
   * can be received.
   *
   * That is, if true, then a call to receive() will return a vector
   * that is not empty.
   */
  virtual bool has_sensor_data() const = 0;

  /**
   * Non-blocking function to return all received OSI messages.
   */
  virtual std::vector<std::shared_ptr<osi3::SensorData>> receive_sensor_data() = 0;

  virtual void to_json(cloe::Json& j) const = 0;

  friend void to_json(cloe::Json& j, const OsiTransceiver& t) { t.to_json(j); }
};

}  // namespace osii
