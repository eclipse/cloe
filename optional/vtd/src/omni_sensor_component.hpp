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
 * \file omni_sensor_component.hpp
 * \see  omni_sensor_component.cpp
 */

#pragma once

#include <limits>  // for numeric_limits<>
#include <map>     // for map<>
#include <memory>  // for shared_ptr<>, unique_ptr<>
#include <string>  // for string, to_string

#include "vtd_version.hpp"
#if (VTD_API_VERSION_EPOCH == 0)
  #include <viRDBIcd.h>  // for RDB_OBJECT_STATE_t, ...
#else
  #include <VtdToolkit/viRDBIcd.h>
#endif

#include "rdb_codec.hpp"        // for RdbCodec, RdbTransceiver
#include "vtd_logger.hpp"       // for vtd_logger
#include "vtd_sensor_data.hpp"  // for VtdSensorData

namespace vtd {

/**
 * Converts a RDB_COORD_t into Eigen::Isometry3d.
 */
Eigen::Isometry3d from_vtd_pose(const RDB_COORD_t& x);

/**
 * Converts a RDB_OBJECT_STATE_t into an Object.
 *
 * \param rdb_object_state pointer to the RDB object state
 * \param extended indicates that rdb_object_state provides extended information
 * \param object object where the converted state information is written in
 */
void from_vtd_object_state(const RDB_OBJECT_STATE_t* rdb_os, bool ext, cloe::Object& obj);

void from_vtd_roadmark(const RDB_ROADMARK_t* rdb_rm, cloe::LaneBoundary& lb);

const uint64_t UNDEFINED_OWNER_ID = std::numeric_limits<uint64_t>::max();

/**
 * VtdOmniSensor implements retrieval of all data sent by the related VTD sensor.
 *
 * This is currently object, ego, and lane boundary sensor data.
 *
 * The object sensor senses box-like objects. The objects are received via RDB
 * and provided as an object list. The ego sensor senses wheel and general ego
 * information. The lane boundary sensor senses roadmarks.
 *
 * In order to distinguish ego from non-ego objects OmniSensor uses the owner_id
 * which is the VTD object id from the vehicle owning the related sensor.
 */
class VtdOmniSensor : public RdbCodec, public VtdSensorData {
 public:
  virtual ~VtdOmniSensor() = default;

  VtdOmniSensor(std::unique_ptr<RdbTransceiver>&& rdb_transceiver, uint64_t owner_id)
      : RdbCodec(std::move(rdb_transceiver)), VtdSensorData("rdb_sensor"), owner_id_(owner_id) {
    ego_object_ = std::make_shared<cloe::Object>();  // NOLINT
  }

  void step(const cloe::Sync& s) override { RdbCodec::step(s.step(), restart_, sensor_data_time_); }

  using RdbCodec::process;

  void process(RDB_START_OF_FRAME_t* /*nullptr*/) override {
    vtd_logger()->trace("VtdOmniSensor: start-of-frame");
    VtdSensorData::clear_cache();
  }

  void process(RDB_END_OF_FRAME_t* /*nullptr*/) override {
    vtd_logger()->trace("VtdOmniSensor: end-of-frame");
    assert(ego_object_ || owner_id_ == UNDEFINED_OWNER_ID);
  }

  void process(RDB_WHEEL_t* rdb_w, bool /*extended*/) override {
    auto wheel_player_id = static_cast<int>(rdb_w->base.playerId);
    if (ego_object_ && ego_object_->id == wheel_player_id && rdb_w->base.id == 0) {
      ego_steering_angle_ = rdb_w->base.steeringAngle;
    }
  }

  void process(RDB_SENSOR_STATE_t* s) override {
    frustum_.fov_h = s->fovHV[0];
    frustum_.fov_v = s->fovHV[1];
    frustum_.offset_h = s->fovOffHV[0];
    frustum_.offset_v = s->fovOffHV[1];
    frustum_.clip_near = s->clipNF[0];
    frustum_.clip_far = s->clipNF[1];
    mount_ = from_vtd_pose(s->pos);
  }

  void process(RDB_OBJECT_STATE_t* rdb_os, bool extended) override {
    // Pick ego from objects and put all other objects to object list.
    switch (rdb_os->base.category) {
      case RDB_OBJECT_CATEGORY_PLAYER: {
        auto obj = std::make_shared<cloe::Object>();
        from_vtd_object_state(rdb_os, extended, *obj);
        if (rdb_os->base.id == owner_id_) {
          // Convert ego velocity and acceleration into vehicle frame coordinates
          obj->velocity = obj->pose.rotation().inverse() * obj->velocity;
          obj->acceleration = obj->pose.rotation().inverse() * obj->acceleration;
          ego_object_ = obj;
        } else {
          // All other drivers:
          world_objects_.push_back(obj);
        }
        break;
      }

      case RDB_OBJECT_CATEGORY_COMMON: {
        auto obj = std::make_shared<cloe::Object>();
        from_vtd_object_state(rdb_os, extended, *obj);
        world_objects_.push_back(obj);
        break;
      }

      case RDB_OBJECT_CATEGORY_SENSOR:
      case RDB_OBJECT_CATEGORY_CAMERA:
      case RDB_OBJECT_CATEGORY_LIGHT_POINT:
      case RDB_OBJECT_CATEGORY_NONE:
      case RDB_OBJECT_CATEGORY_OPENDRIVE: {
        vtd_logger()->trace("Discarding object with category {}.", rdb_os->base.category);
        break;
      }

      default: {
        auto category_str = std::to_string(rdb_os->base.category);
        throw std::logic_error("unknown RDB base category " + category_str);
      }
    }
  }

  void process(RDB_ROADMARK_t* rdb_rm) override {
    if (rdb_rm->playerId == owner_id_) {
      auto& lb = lanes_[rdb_rm->id];
      from_vtd_roadmark(rdb_rm, lb);
    }
  }

  const std::string& get_name() const override { return name_; }

  // As defined in `cloe/component.hpp`
  void reset() override {
    VtdSensorData::clear_cache();
    this->set_reset_state();
  }

  friend void to_json(cloe::Json& j, const VtdOmniSensor& s) {
    to_json(j, static_cast<const VtdSensorData&>(s));
    j = cloe::Json{
        {"frame_number", s.frame_number()},
        {"rdb_connection", s.rdb_},
    };
  }

 protected:
  /// Id of the sensor's owner (ego).
  uint64_t owner_id_;
};

}  // namespace vtd
