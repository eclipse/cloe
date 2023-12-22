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
 * \file osi_transceiver_tcp.cpp
 */

#include "osi/utility/osi_transceiver_tcp.hpp"

#include <cassert>  // for assert
#include <cstdlib>  // for malloc, free
#include <memory>   // for shared_ptr<>

#include <cloe/core.hpp>                     // for Error
#include <cloe/utility/tcp_transceiver.hpp>  // for TcpReadError

#include <google/protobuf/io/coded_stream.h>                // for CodedInputStream
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>  // for ArrayInputStream

#include "osi_sensordata.pb.h"  // for SensorData

namespace cloeosi {

// This method reads a complete SensorData struct from the stream.
//
// First, we read the header of the message to find out how much memory we have to allocate,
// and then we read the rest of the data after verifying the validity of the header.
std::shared_ptr<osi3::SensorData> OsiTransceiverTcp::receive_sensor_data_wait() {
  // 1. Read the header (= data_size).
  auto hdr_buf = reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t)));
  tcp_stream_.read(reinterpret_cast<char*>(hdr_buf), sizeof(uint32_t));
  if (!tcp_stream_) {
    free(hdr_buf);
    this->num_errors_++;
    throw cloe::utility::TcpReadError("OsiTransceiverTcp: error during header read: {}",
                                      tcp_stream_.error().message());
  }

  // Parse as protobuf for consistency with send().
  google::protobuf::io::ArrayInputStream hdr_array_input(hdr_buf, sizeof(uint32_t));
  google::protobuf::io::CodedInputStream hdr_code_input(&hdr_array_input);

  uint32_t data_size = 0;
  hdr_code_input.ReadLittleEndian32(&data_size);
  free(hdr_buf);

  // 2.a) Allocate enough memory for the message.
  auto data_buf = reinterpret_cast<char*>(malloc(data_size));
  if (data_buf == nullptr) {
    throw cloe::Error("OsiTransceiverTcp: malloc failed");
  }

  // 2.b) Read the message.
  tcp_stream_.read(data_buf, data_size);
  if (!tcp_stream_) {
    free(data_buf);
    this->num_errors_++;
    throw cloe::utility::TcpReadError("OsiTransceiverTcp: error during read: {}",
                                      tcp_stream_.error().message());
  }

  // 3. Parse the data message as protobuf input stream.
  google::protobuf::io::ArrayInputStream array_input(data_buf, data_size);
  google::protobuf::io::CodedInputStream code_input(&array_input);
  google::protobuf::io::CodedInputStream::Limit pb_limit = code_input.PushLimit(data_size);

  auto sensor_data_rcv = std::make_shared<osi3::SensorData>();
  if (!sensor_data_rcv->ParseFromCodedStream(&code_input)) {
    throw OsiError("OsiTransceiverTcp: failure while parsing osi message");
  }
  code_input.PopLimit(pb_limit);

  // 4. Cleanup.
  free(data_buf);

  // 5. Consistency checks.
  if (sensor_data_rcv->ByteSizeLong() != static_cast<size_t>(data_size)) {
    this->num_errors_++;
    throw OsiError("OsiTransceiverTcp: inconsistent data size in osi protobuf message");
  }

  if (!sensor_data_rcv->IsInitialized()) {
    this->num_errors_++;
    throw OsiError(
        "OsiTransceiverTcp: incoming osi sensor_data message was not correctly initialized");
  }

  // 5. Wrap the result in a shared_ptr and return it.
  return sensor_data_rcv;
}

}  // namespace cloeosi
