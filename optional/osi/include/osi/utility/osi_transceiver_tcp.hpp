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
 * \file osi_transceiver_tcp.hpp
 * \see  osi_transceiver_tcp.cpp
 */

#pragma once

#include <memory>  // for shared_ptr<>
#include <string>  // for string
#include <vector>  // for vector<>

#include <boost/asio.hpp>  // for streamsize

#include <cloe/core.hpp>                     // for Json, Logger
#include <cloe/utility/tcp_transceiver.hpp>  // for TcpTransceiver

#include "osi_groundtruth.pb.h"  // for GroundTruth
#include "osi_sensordata.pb.h"   // for SensorData

#include "osi/utility/osi_transceiver.hpp"  // for OsiTransceiver
#include "osi/utility/osi_utils.hpp"        // for osi_logger

namespace cloeosi {

/**
 * OsiTransceiverTcp implements an OsiTransceiver via TCP.
 */
class OsiTransceiverTcp : public OsiTransceiver, public cloe::utility::TcpTransceiver {
 public:
  using TcpTransceiver::TcpTransceiver;

  bool has_sensor_data() const override {
    return this->tcp_available_data() >= static_cast<std::streamsize>(sizeof(osi3::SensorData));
  }

  bool has_sensor_view() const override {
    return this->tcp_available_data() >= static_cast<std::streamsize>(sizeof(osi3::SensorView));
  }

  bool has_ground_truth() const override {
    return this->tcp_available_data() >= static_cast<std::streamsize>(sizeof(osi3::GroundTruth));
  }

  void receive_osi_msgs(std::vector<std::shared_ptr<osi3::SensorData>>& msgs) override {
    if (msgs.size() > 0) {
      osi_logger()->warn(
          "OsiTransceiverTcp: Non-zero length of message vector before retrieval: {}", msgs.size());
    }
    while (this->has_sensor_data()) {
      num_received_++;
      msgs.push_back(this->receive_sensor_data_wait());
    }
  }

  void receive_osi_msgs(std::vector<std::shared_ptr<osi3::SensorView>>& /*msgs*/) override {
    throw OsiError("OsiTransceiverTcp: Retrieval of osi3::SensorView not yet implemented.");
  }

  void receive_osi_msgs(std::vector<std::shared_ptr<osi3::GroundTruth>>& /*msgs*/) override {
    throw OsiError("OsiTransceiverTcp: Retrieval of osi3::GroundTruth not yet implemented.");
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

  friend void to_json(cloe::Json& j, const OsiTransceiverTcp& t) { t.to_json(j); }

 protected:
  /**
   * Synchronous (blocking) method to receive a SensorData message.
   */
  std::shared_ptr<osi3::SensorData> receive_sensor_data_wait();

 private:
  // Statistics for interest's sake
  uint64_t num_errors_{0};
  uint64_t num_sent_{0};
  uint64_t num_received_{0};
};

class OsiTransceiverTcpFactory : public cloe::utility::TcpTransceiverFactory<OsiTransceiverTcp> {
 public:
  using TcpTransceiverFactory::TcpTransceiverFactory;

 protected:
  cloe::Logger factory_logger() const override { return osi_logger(); }
  const char* instance_name() const override { return "OsiTransceiverTcp"; }
};

}  // namespace cloeosi
