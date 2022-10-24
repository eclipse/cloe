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
 * \file rdb_codec.hpp
 * \see  rdb_codec.cpp
 */

#pragma once

#include <map>      // for map<>
#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move

// This comes from VTD:
#include <RDBHandler.hh>

#include <cloe/core.hpp>  // for Json, Duration

#include "rdb_transceiver.hpp"  // for RdbTransceiver

namespace vtd {

/**
 * Base class for a VTD sensor which is connected via RDB.
 */
class RdbCodec {
 public:
  virtual ~RdbCodec() = default;
  RdbCodec() = delete;

  /**
   * Create a new instance of RdbCodec with the given RdbTransceiver.
   */
  explicit RdbCodec(std::unique_ptr<RdbTransceiver>&& rdb_transceiver)
      : rdb_(std::move(rdb_transceiver)) {}

  /**
   * Create a new instance of RdbCodec with the given new RdbTransceiver.
   *
   * WARNING: If you use this constructor, please realize that RdbCodec
   *          takes ownership of the pointer you pass in.
   */
  explicit RdbCodec(RdbTransceiver* rdb_transceiver) : rdb_(rdb_transceiver) {}

  /**
   * Return the Codec's or Sensors' name.
   * Will be overwritten by the name set in VtdSensorData.
   */
  virtual const std::string& get_name() const = 0;

  /**
   * Return the last processed frame number, 0 if no frames have been processed.
   */
  virtual uint64_t frame_number() const { return frame_number_; }

  /**
   * Receive and process the incoming messages.
   */
  virtual void step(uint64_t frame_number, bool& restart, cloe::Duration& sim_time);

  /**
   * Process a RDB message.
   * This method is called in process() for each RDB message.
   *
   * \param msg message to be processed
   */
  virtual void process(RDB_MSG_t* msg, bool& restart, cloe::Duration& sim_time);

  /**
   * Process each RDB message entry.
   * This method is called in process(RDB_MSG_t) for each entry in a message.
   *
   * \param entry RDB message entry to be processed
   */
  virtual void process(RDB_MSG_ENTRY_HDR_t* entry);

  /**
   * Process a start of frame.
   *
   * \param start_of_frame only used for dynamic binding; do not access.
   */
  virtual void process(RDB_START_OF_FRAME_t* /*start_of_frame*/) {}

  virtual void process(RDB_END_OF_FRAME_t* /*end_of_frame*/) {}
  virtual void process(RDB_COORD_SYSTEM_t* /*coord_system*/) {}
  virtual void process(RDB_COORD_t* /*coord*/) {}
  virtual void process(RDB_ROAD_POS_t* /*road_pos*/) {}
  virtual void process(RDB_LANE_INFO_t* /*lane_info*/) {}
  virtual void process(RDB_ROADMARK_t* /*roadmark*/) {}
  virtual void process(RDB_OBJECT_CFG_t* /*object_cfg*/) {}
  virtual void process(RDB_OBJECT_STATE_t* /*object_state*/, bool /*extended*/) {}
  virtual void process(RDB_VEHICLE_SYSTEMS_t* /*vehicle_systems*/) {}
  virtual void process(RDB_VEHICLE_SETUP_t* /*vehicle_setup*/) {}
  virtual void process(RDB_ENGINE_t* /*engine*/, bool /*extended*/) {}
  virtual void process(RDB_DRIVETRAIN_t* /*drivetrain*/, bool /*extended*/) {}
  virtual void process(RDB_WHEEL_t* /*wheel*/, bool /*extended*/) {}
  virtual void process(RDB_PED_ANIMATION_t* /*ped_animation*/) {}
  virtual void process(RDB_SENSOR_STATE_t* /*sensor_state*/) {}
  virtual void process(RDB_SENSOR_OBJECT_t* /*sensor_object*/) {}
  virtual void process(RDB_CAMERA_t* /*camera*/) {}
  virtual void process(RDB_CONTACT_POINT_t* /*contact_point*/) {}
  virtual void process(RDB_TRAFFIC_SIGN_t* /*traffic_sign*/) {}
  virtual void process(RDB_ROAD_STATE_t* /*road_state*/) {}
  virtual void process(RDB_IMAGE_t* /*image*/) {}
  virtual void process(RDB_LIGHT_SOURCE_t* /*light_source*/, bool /*extended*/) {}
  virtual void process(RDB_ENVIRONMENT_t* /*environment*/) {}
  virtual void process(RDB_TRIGGER_t* /*trigger*/) {}
  virtual void process(RDB_DRIVER_CTRL_t* /*driver_ctrl*/) {}
  virtual void process(RDB_TRAFFIC_LIGHT_t* /*traffic_light*/, bool /*extended*/) {}
  virtual void process(RDB_SYNC_t* /*sync*/) {}
  virtual void process(RDB_DRIVER_PERCEPTION_t* /*driver_perception*/) {}
  virtual void process(RDB_FUNCTION_t* /*function*/) {}
  virtual void process(RDB_ROAD_QUERY_t* /*road_query*/) {}
  virtual void process(RDB_TRAJECTORY_t* /*trajectory*/) {}
  virtual void process(RDB_DYN_2_STEER_t* /*dyn_to_steer*/) {}
  virtual void process(RDB_STEER_2_DYN_t* /*steer_to_dyn*/) {}
  virtual void process(RDB_PROXY_t* /*proxy*/) {}
  virtual void process(RDB_MOTION_SYSTEM_t* /*motion_system*/) {}
  virtual void process(RDB_FREESPACE_t* /*freepsace*/) {}
  virtual void process(RDB_DYN_EL_SWITCH_t* /*dyn_el_switch*/) {}
  virtual void process(RDB_DYN_EL_DOF_t* /*dyn_el_dof*/) {}
  virtual void process(RDB_IG_FRAME_t* /*ig_frame*/) {}
  virtual void process(RDB_RT_PERFORMANCE_t* /*rt_performance*/) {}
  virtual void process(RDB_CUSTOM_SCORING_t* /*custom_scoring*/) {}
  virtual void process(RDB_CUSTOM_OBJECT_CTRL_TRACK_t* /*custom_object_ctrl_track*/) {}

  friend void to_json(cloe::Json& j, const RdbCodec& c) {
    j = cloe::Json{
        {"rdb_connection", *c.rdb_},
        {"frame_number", c.frame_number_},
    };
  }

 protected:
  /// Connection via RDB bus, e.g., TCP, SHM, to VTD.
  /// Should always be valid.
  std::unique_ptr<RdbTransceiver> rdb_;

  /// Frame number from last processed RDB message.
  uint64_t frame_number_ = 0;

  /// Indicates whether in between a start of frame and an end of frame message.
  bool processing_frame_ = false;
};

inline std::string vtd_pkg_id_to_string(uint16_t vtd_pkg_id) {
  // clang-format off
  static const std::map<int, const char*> vtd_pkg_id_name_map{
      {RDB_PKG_ID_START_OF_FRAME, "RDB_PKG_ID_START_OF_FRAME"},
      {RDB_PKG_ID_END_OF_FRAME, "RDB_PKG_ID_END_OF_FRAME"},
      {RDB_PKG_ID_COORD_SYSTEM, "RDB_PKG_ID_COORD_SYSTEM"},
      {RDB_PKG_ID_COORD, "RDB_PKG_ID_COORD"},
      {RDB_PKG_ID_ROAD_POS, "RDB_PKG_ID_ROAD_POS"},
      {RDB_PKG_ID_LANE_INFO, "RDB_PKG_ID_LANE_INFO"},
      {RDB_PKG_ID_ROADMARK, "RDB_PKG_ID_ROADMARK"},
      {RDB_PKG_ID_OBJECT_CFG, "RDB_PKG_ID_OBJECT_CFG"},
      {RDB_PKG_ID_OBJECT_STATE, "RDB_PKG_ID_OBJECT_STATE"},
      {RDB_PKG_ID_VEHICLE_SYSTEMS, "RDB_PKG_ID_VEHICLE_SYSTEMS"},
      {RDB_PKG_ID_VEHICLE_SETUP, "RDB_PKG_ID_VEHICLE_SETUP"},
      {RDB_PKG_ID_ENGINE, "RDB_PKG_ID_ENGINE"},
      {RDB_PKG_ID_DRIVETRAIN, "RDB_PKG_ID_DRIVETRAIN"},
      {RDB_PKG_ID_WHEEL, "RDB_PKG_ID_WHEEL"},
      {RDB_PKG_ID_PED_ANIMATION, "RDB_PKG_ID_PED_ANIMATION"},
      {RDB_PKG_ID_SENSOR_STATE, "RDB_PKG_ID_SENSOR_STATE"},
      {RDB_PKG_ID_SENSOR_OBJECT, "RDB_PKG_ID_SENSOR_OBJECT"},
      {RDB_PKG_ID_CAMERA, "RDB_PKG_ID_CAMERA"},
      {RDB_PKG_ID_CONTACT_POINT, "RDB_PKG_ID_CONTACT_POINT"},
      {RDB_PKG_ID_TRAFFIC_SIGN, "RDB_PKG_ID_TRAFFIC_SIGN"},
      {RDB_PKG_ID_ROAD_STATE, "RDB_PKG_ID_ROAD_STATE"},
      {RDB_PKG_ID_IMAGE, "RDB_PKG_ID_IMAGE"},
      {RDB_PKG_ID_LIGHT_SOURCE, "RDB_PKG_ID_LIGHT_SOURCE"},
      {RDB_PKG_ID_ENVIRONMENT, "RDB_PKG_ID_ENVIRONMENT"},
      {RDB_PKG_ID_TRIGGER, "RDB_PKG_ID_TRIGGER"},
      {RDB_PKG_ID_DRIVER_CTRL, "RDB_PKG_ID_DRIVER_CTRL"},
      {RDB_PKG_ID_TRAFFIC_LIGHT, "RDB_PKG_ID_TRAFFIC_LIGHT"},
      {RDB_PKG_ID_SYNC, "RDB_PKG_ID_SYNC"},
      {RDB_PKG_ID_DRIVER_PERCEPTION, "RDB_PKG_ID_DRIVER_PERCEPTION"},
      {RDB_PKG_ID_LIGHT_MAP, "RDB_PKG_ID_LIGHT_MAP"},
      {RDB_PKG_ID_TONE_MAPPING, "RDB_PKG_ID_TONE_MAPPING"},
      {RDB_PKG_ID_ROAD_QUERY, "RDB_PKG_ID_ROAD_QUERY"},
      {RDB_PKG_ID_SCP, "RDB_PKG_ID_SCP"},
      {RDB_PKG_ID_TRAJECTORY, "RDB_PKG_ID_TRAJECTORY"},
      {RDB_PKG_ID_DYN_2_STEER, "RDB_PKG_ID_DYN_2_STEER"},
      {RDB_PKG_ID_STEER_2_DYN, "RDB_PKG_ID_STEER_2_DYN"},
      {RDB_PKG_ID_PROXY, "RDB_PKG_ID_PROXY"},
      {RDB_PKG_ID_MOTION_SYSTEM, "RDB_PKG_ID_MOTION_SYSTEM"},
      {RDB_PKG_ID_OCCLUSION_MATRIX, "RDB_PKG_ID_OCCLUSION_MATRIX"},
      {RDB_PKG_ID_FREESPACE, "RDB_PKG_ID_FREESPACE"},
      {RDB_PKG_ID_DYN_EL_SWITCH, "RDB_PKG_ID_DYN_EL_SWITCH"},
      {RDB_PKG_ID_DYN_EL_DOF, "RDB_PKG_ID_DYN_EL_DOF"},
      {RDB_PKG_ID_IG_FRAME, "RDB_PKG_ID_IG_FRAME"},
      {RDB_PKG_ID_RAY, "RDB_PKG_ID_RAY"},
      {RDB_PKG_ID_RT_PERFORMANCE, "RDB_PKG_ID_RT_PERFORMANCE"},
      {RDB_PKG_ID_CUSTOM_SCORING, "RDB_PKG_ID_CUSTOM_SCORING"},
      {RDB_PKG_ID_CUSTOM_OBJECT_CTRL_TRACK, "RDB_PKG_ID_CUSTOM_OBJECT_CTRL_TRACK"},
      {RDB_PKG_ID_CUSTOM_LIGHT_B, "RDB_PKG_ID_CUSTOM_LIGHT_B"},
      {RDB_PKG_ID_CUSTOM_LIGHT_A, "RDB_PKG_ID_CUSTOM_LIGHT_A"},
      {RDB_PKG_ID_CUSTOM_LIGHT_GROUP_B, "RDB_PKG_ID_CUSTOM_LIGHT_GROUP_B"},
#if RDB_VERSION >= 0x011E
      {RDB_PKG_ID_CUSTOM_LOOK_AHEAD, "RDB_PKG_ID_CUSTOM_LOOK_AHEAD"},
#endif
      {RDB_PKG_ID_CUSTOM_AUDI_FORUM, "RDB_PKG_ID_CUSTOM_AUDI_FORUM"},
      {RDB_PKG_ID_CUSTOM_OPTIX_START, "RDB_PKG_ID_CUSTOM_OPTIX_START"},
      {RDB_PKG_ID_OPTIX_BUFFER, "RDB_PKG_ID_OPTIX_BUFFER"},
      {RDB_PKG_ID_CUSTOM_OPTIX_END, "RDB_PKG_ID_CUSTOM_OPTIX_END"},
      {RDB_PKG_ID_CUSTOM_USER_A_START, "RDB_PKG_ID_CUSTOM_USER_A_START"},
      {RDB_PKG_ID_CUSTOM_USER_A_END, "RDB_PKG_ID_CUSTOM_USER_A_END"},
      {RDB_PKG_ID_CUSTOM_USER_B_START, "RDB_PKG_ID_CUSTOM_USER_B_START"},
      {RDB_PKG_ID_CUSTOM_USER_B_END, "RDB_PKG_ID_CUSTOM_USER_B_END"}};
  // clang-format on
  return vtd_pkg_id_name_map.at(vtd_pkg_id);
}

}  // namespace vtd
