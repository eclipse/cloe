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
 * \file rdb_codec.cpp
 */

#include "rdb_codec.hpp"

#include <cassert>  // for assert

#include <cloe/core.hpp>  // for Duration

#include "vtd_logger.hpp"  // for rdb_logger

namespace vtd {

void RdbCodec::process(RDB_MSG_ENTRY_HDR_t* entry) {
  if (entry == nullptr) {
    return;
  }

  char* data = nullptr;
  if (entry->elementSize == 0) {
    switch (entry->pkgId) {
      case RDB_PKG_ID_START_OF_FRAME:
        processing_frame_ = true;
        // Note: yes, it's a nullptr, but it's the only way to force the
        // compiler to choose the correct process method.
        process(reinterpret_cast<RDB_START_OF_FRAME_t*>(data));
        break;

      case RDB_PKG_ID_END_OF_FRAME:
        processing_frame_ = false;
        // Note: yes, it's a nullptr, but it's the only way to force the
        // compiler to choose the correct process method.
        process(reinterpret_cast<RDB_END_OF_FRAME_t*>(data));
        break;

      default:
        rdb_logger()->error("RdbCodec: RDB package ID '{}' not implemented", entry->pkgId);
        break;
    }
    return;
  }

  size_t number_elements = entry->dataSize / entry->elementSize;
  assert(number_elements * entry->elementSize == entry->dataSize);
  data = reinterpret_cast<char*>(entry) + entry->headerSize;

  for (size_t i = 0; i < number_elements; ++i) {
    switch (entry->pkgId) {
      case RDB_PKG_ID_COORD_SYSTEM:
        process(reinterpret_cast<RDB_COORD_SYSTEM_t*>(data));
        break;

      case RDB_PKG_ID_COORD:
        process(reinterpret_cast<RDB_COORD_t*>(data));
        break;

      case RDB_PKG_ID_ROAD_POS:
        process(reinterpret_cast<RDB_ROAD_POS_t*>(data));
        break;

      case RDB_PKG_ID_LANE_INFO:
        process(reinterpret_cast<RDB_LANE_INFO_t*>(data));
        break;

      case RDB_PKG_ID_ROADMARK:
        process(reinterpret_cast<RDB_ROADMARK_t*>(data));
        break;

      case RDB_PKG_ID_OBJECT_CFG:
        process(reinterpret_cast<RDB_OBJECT_CFG_t*>(data));
        break;

      case RDB_PKG_ID_OBJECT_STATE:
        process(reinterpret_cast<RDB_OBJECT_STATE_t*>(data), entry->flags & RDB_PKG_FLAG_EXTENDED);
        break;

      case RDB_PKG_ID_VEHICLE_SYSTEMS:
        process(reinterpret_cast<RDB_VEHICLE_SYSTEMS_t*>(data));
        break;

      case RDB_PKG_ID_VEHICLE_SETUP:
        process(reinterpret_cast<RDB_VEHICLE_SETUP_t*>(data));
        break;

      case RDB_PKG_ID_ENGINE:
        process(reinterpret_cast<RDB_ENGINE_t*>(data), entry->flags & RDB_PKG_FLAG_EXTENDED);
        break;

      case RDB_PKG_ID_DRIVETRAIN:
        process(reinterpret_cast<RDB_DRIVETRAIN_t*>(data), entry->flags & RDB_PKG_FLAG_EXTENDED);
        break;

      case RDB_PKG_ID_WHEEL:
        process(reinterpret_cast<RDB_WHEEL_t*>(data), entry->flags & RDB_PKG_FLAG_EXTENDED);
        break;

      case RDB_PKG_ID_PED_ANIMATION:
        process(reinterpret_cast<RDB_PED_ANIMATION_t*>(data));
        break;

      case RDB_PKG_ID_SENSOR_STATE:
        process(reinterpret_cast<RDB_SENSOR_STATE_t*>(data));
        break;

      case RDB_PKG_ID_SENSOR_OBJECT:
        process(reinterpret_cast<RDB_SENSOR_OBJECT_t*>(data));
        break;

      case RDB_PKG_ID_CAMERA:
        process(reinterpret_cast<RDB_CAMERA_t*>(data));
        break;

      case RDB_PKG_ID_CONTACT_POINT:
        process(reinterpret_cast<RDB_CONTACT_POINT_t*>(data));
        break;

      case RDB_PKG_ID_TRAFFIC_SIGN:
        process(reinterpret_cast<RDB_TRAFFIC_SIGN_t*>(data));
        break;

      case RDB_PKG_ID_ROAD_STATE:
        process(reinterpret_cast<RDB_ROAD_STATE_t*>(data));
        break;

      case RDB_PKG_ID_IMAGE:
      case RDB_PKG_ID_LIGHT_MAP:
        process(reinterpret_cast<RDB_IMAGE_t*>(data));
        break;

      case RDB_PKG_ID_OCCLUSION_MATRIX:
        throw std::runtime_error("RdbCodec: RDB_PKG_ID_OCCLUSION_MATRIX not implemented");
        break;

      case RDB_PKG_ID_LIGHT_SOURCE:
        process(reinterpret_cast<RDB_LIGHT_SOURCE_t*>(data), entry->flags & RDB_PKG_FLAG_EXTENDED);
        break;

      case RDB_PKG_ID_ENVIRONMENT:
        process(reinterpret_cast<RDB_ENVIRONMENT_t*>(data));
        break;

      case RDB_PKG_ID_TRIGGER:
        process(reinterpret_cast<RDB_TRIGGER_t*>(data));
        break;

      case RDB_PKG_ID_DRIVER_CTRL:
        process(reinterpret_cast<RDB_DRIVER_CTRL_t*>(data));
        break;

      case RDB_PKG_ID_TRAFFIC_LIGHT:
        process(reinterpret_cast<RDB_TRAFFIC_LIGHT_t*>(data), entry->flags & RDB_PKG_FLAG_EXTENDED);
        break;

      case RDB_PKG_ID_SYNC:
        process(reinterpret_cast<RDB_SYNC_t*>(data));
        break;

      case RDB_PKG_ID_DRIVER_PERCEPTION:
        process(reinterpret_cast<RDB_DRIVER_PERCEPTION_t*>(data));
        break;

      case RDB_PKG_ID_TONE_MAPPING:
        process(reinterpret_cast<RDB_FUNCTION_t*>(data));
        break;

      case RDB_PKG_ID_ROAD_QUERY:
        process(reinterpret_cast<RDB_ROAD_QUERY_t*>(data));
        break;

      case RDB_PKG_ID_SCP:
        throw std::runtime_error("RdbCodec: RDB_PKG_ID_SCP not implemented");
        break;

      case RDB_PKG_ID_TRAJECTORY:
        process(reinterpret_cast<RDB_TRAJECTORY_t*>(data));
        break;

      case RDB_PKG_ID_DYN_2_STEER:
        process(reinterpret_cast<RDB_DYN_2_STEER_t*>(data));
        break;

      case RDB_PKG_ID_STEER_2_DYN:
        process(reinterpret_cast<RDB_STEER_2_DYN_t*>(data));
        break;

      case RDB_PKG_ID_PROXY:
        process(reinterpret_cast<RDB_PROXY_t*>(data));
        break;

      case RDB_PKG_ID_MOTION_SYSTEM:
        process(reinterpret_cast<RDB_MOTION_SYSTEM_t*>(data));
        break;

      case RDB_PKG_ID_FREESPACE:
        process(reinterpret_cast<RDB_FREESPACE_t*>(data));
        break;

      case RDB_PKG_ID_DYN_EL_SWITCH:
        process(reinterpret_cast<RDB_DYN_EL_SWITCH_t*>(data));
        break;

      case RDB_PKG_ID_DYN_EL_DOF:
        process(reinterpret_cast<RDB_DYN_EL_DOF_t*>(data));
        break;

      case RDB_PKG_ID_IG_FRAME:
        process(reinterpret_cast<RDB_IG_FRAME_t*>(data));
        break;

      case RDB_PKG_ID_RT_PERFORMANCE:
        process(reinterpret_cast<RDB_RT_PERFORMANCE_t*>(data));
        break;

      case RDB_PKG_ID_CUSTOM_SCORING:
        process(reinterpret_cast<RDB_CUSTOM_SCORING_t*>(data));
        break;

      case RDB_PKG_ID_CUSTOM_OBJECT_CTRL_TRACK:
        process(reinterpret_cast<RDB_CUSTOM_OBJECT_CTRL_TRACK_t*>(data));
        break;

      default:
        rdb_logger()->error("RdbCodec: RDB package ID '{}' not implemented", entry->pkgId);
        break;
    }
    data = reinterpret_cast<char*>(data) + entry->elementSize;
  }
  assert(reinterpret_cast<char*>(entry) + entry->headerSize + entry->dataSize == data);
}

void RdbCodec::process(RDB_MSG_t* msg, bool& restart, cloe::Duration& sim_time) {
  RDB_MSG_ENTRY_HDR_t* entry = nullptr;
  uint32_t remaining_bytes = 0;

  if (msg == nullptr) {
    return;
  }

  if (0 == msg->hdr.dataSize) {
    return;
  }

  frame_number_ = msg->hdr.frameNo;

  if (restart && frame_number_ != 0) {
    rdb_logger()->debug("RdbCodec: discarding RDB message [restart, frame={}]", frame_number_);
    return;
  }

  if (frame_number_ == 0) {
    restart = false;
  }

  sim_time =
      std::chrono::duration_cast<cloe::Duration>(std::chrono::duration<float>(msg->hdr.simTime));
  rdb_logger()->trace("RdbCodec: message frame {} @ {} ns", frame_number_, sim_time.count());
  entry =
      reinterpret_cast<RDB_MSG_ENTRY_HDR_t*>(reinterpret_cast<char*>(msg) + msg->hdr.headerSize);
  remaining_bytes = msg->hdr.dataSize;

  while (remaining_bytes > 0) {
    rdb_logger()->trace("[{:12s}]   Frame {} @ {} ns --> {}", get_name(), frame_number_,
                        sim_time.count(), vtd_pkg_id_to_string(entry->pkgId));
    process(entry);
    remaining_bytes -= (entry->headerSize + entry->dataSize);
    entry = reinterpret_cast<RDB_MSG_ENTRY_HDR_t*>(reinterpret_cast<char*>(entry) +
                                                   entry->headerSize + entry->dataSize);
  }
  assert(remaining_bytes == 0);
}

void RdbCodec::step(uint64_t frame_number, bool& restart, cloe::Duration& sim_time) {
  // TODO(ben): For some reason, this loop here goes round and round with zero
  // messages received. Either we should stop dumping the log message when
  // there are no messages to process, or we should have the receive function
  // only return when there are messages?
  while (processing_frame_ || frame_number_ < frame_number || restart) {
    auto messages = rdb_->receive();
    rdb_logger()->trace("RdbCodec: processing {} messages [frame={}]", messages.size(),
                        frame_number_);
    for (auto m : messages) {
      this->process(m.get(), restart, sim_time);
    }
  }
  assert(frame_number_ == frame_number &&
         "VTD frame number exceeds frame number expected by Cloe!");
  rdb_logger()->trace("RdbCodec: completed processing messages [frame={}]", frame_number_);
}

}  // namespace vtd
