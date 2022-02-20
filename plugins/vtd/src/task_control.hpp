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
 * \file task_control.hpp
 */

#pragma once

#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move

#include <RDBHandler.hh>

#include <cloe/component/object.hpp>  // for Object
#include <cloe/core.hpp>              // for Json

#include "omni_sensor_component.hpp"  // for VtdOmniSensor
#include "rdb_codec.hpp"              // for RdbCodec
#include "vtd_logger.hpp"             // for vtd_logger

namespace vtd {

struct DriverControl {
  /// VTD player ID.
  uint32_t player_id{0};

  /// Target acceleration in [m/s^2].
  float target_acceleration{0};

  /// Target steering angle in [rad].
  float target_steering{0};

  /**
   * A combination of:
   * - RDB_DRIVER_FLAG_INDICATOR_L
   * - RDB_DRIVER_FLAG_INDICATOR_R
   * - RDB_DRIVER_FLAG_PARKING_BRAKE
   */
  uint32_t driver_flags{0};

  /**
   * A combination of:
   * - RDB_DRIVER_INPUT_VALIDITY_TGT_STEERING
   * - RDB_DRIVER_INPUT_VALIDITY_TGT_ACCEL
   * - RDB_DRIVER_INPUT_VALIDITY_ADD_ON
   * - RDB_DRIVER_INPUT_VALIDITY_FLAGS
   */
  uint32_t validity_flags{0};

  friend void to_json(cloe::Json& j, const DriverControl& dc) {
    j = cloe::Json{
        {"player_id", dc.player_id},
        {"target_acceleration", dc.target_acceleration},
        {"target_steering", dc.target_steering},
        {"driver_flags", dc.driver_flags},
        {"validity_flags", dc.validity_flags},
    };
  }
};

struct DynObjectState {
  /// Object id.
  uint32_t base_id{0};

  /// Object category (player, sensor, ...).
  uint8_t base_category{RDB_OBJECT_CATEGORY_PLAYER};

  /// Object type (car, truck, ...).
  uint8_t base_type{RDB_OBJECT_TYPE_NONE};

  /// Visibility mask (e.g. visible for traffic and visible for data recorder).
  uint16_t base_vis_mask{RDB_OBJECT_VIS_FLAG_TRAFFIC | RDB_OBJECT_VIS_FLAG_RECORDER};

  /// Player name.
  std::string base_name;

  /// Object dimension and offset to cog.
  RDB_GEOMETRY_t base_geo;

  /// Object position and orientation.
  RDB_COORD_t base_pos;

  /// Object velocity and angular velocity.
  RDB_COORD_t ext_speed;

  /// Object acceleration and angular acceleration.
  RDB_COORD_t ext_accel;

  friend void to_json(cloe::Json& j, const DynObjectState& os) {
    j = cloe::Json{
        {"base_id", os.base_id},     {"base_category", os.base_category},
        {"base_type", os.base_type}, {"base_vis_mask", os.base_vis_mask},
        {"base_name", os.base_name},
    };
  }
};

/**
 * Map to convert from Cloe to VTD object classification.
 */
const std::map<cloe::Object::Class, uint8_t> cloe_vtd_obj_class_map = {
    {cloe::Object::Class::Car, RDB_OBJECT_TYPE_PLAYER_CAR},
    {cloe::Object::Class::Truck, RDB_OBJECT_TYPE_PLAYER_TRUCK},
    {cloe::Object::Class::Motorbike, RDB_OBJECT_TYPE_PLAYER_MOTORBIKE},
    {cloe::Object::Class::Trailer, RDB_OBJECT_TYPE_PLAYER_TRAILER},
};

/**
 * Convert object geometry VTD geometry.
 */
RDB_GEOMETRY_t rdb_geometry_from_object(const cloe::Object& obj) {
  RDB_GEOMETRY_t geo;
  geo.dimX = obj.dimensions.x();
  geo.dimY = obj.dimensions.y();
  geo.dimZ = obj.dimensions.z();
  geo.offX = obj.cog_offset.x();
  geo.offY = obj.cog_offset.y();
  geo.offZ = obj.cog_offset.z();
  return geo;
}

RDB_COORD_t rdb_coord_from_vector3d(const Eigen::Vector3d& position,
                                    const Eigen::Vector3d& angle_rph) {
  RDB_COORD_t coord;
  coord.x = position.x();
  coord.y = position.y();
  coord.z = position.z();
  coord.r = angle_rph.x();
  coord.p = angle_rph.y();
  coord.h = angle_rph.z();
  coord.flags = RDB_COORD_FLAG_POINT_VALID | RDB_COORD_FLAG_ANGLES_VALID;
  coord.type = RDB_COORD_TYPE_INERTIAL;
  return coord;
}

RDB_COORD_t rdb_coord_from_object(const cloe::Object& obj) {
  Eigen::Vector3d hpr = obj.pose.rotation().matrix().eulerAngles(2, 1, 0);
  return rdb_coord_from_vector3d(obj.pose.translation(),
                                 Eigen::Vector3d(hpr.z(), hpr.y(), hpr.x()));
}

RDB_COORD_t rdb_coord_pos_from_vector3d(const Eigen::Vector3d& position) {
  RDB_COORD_t coord;
  coord.x = position.x();
  coord.y = position.y();
  coord.z = position.z();
  coord.flags = RDB_COORD_FLAG_POINT_VALID;
  coord.type = RDB_COORD_TYPE_INERTIAL;
  return coord;
}

/**
 * TaskControl contains the connection to the VTD task control server.
 *
 * In the usual basic single machine configurations of VTD, there is only one
 * instance of task control server running.
 *
 * This class implements
 * - Receiving groundtruth data (e.g. object data in inertial coordinates)
 * - Sending all vehicle actuations
 * - Triggering VTD simulation time steps
 *
 * The idea is that the VtdOmniSensor baseclass receives all groundtruth data
 * and exposes it at the default cloe groundtruth sensor's component-interfaces.
 * Then you add any packages to the task control that you want to send, and
 * once you've done that, you send the packages, which packs the buffer and
 * sends it off.
 *
 * Contrived example:
 *
 *    TaskControl tc{new RdbTransceiver("localhost", 23456)};
 *
 *    this->new_component(new VtdWorldSensor{tc},
                        CloeComponent::GROUNDTRUTH_WORLD_SENSOR);
 *    ...
 *    DriverControl dc;
 *    dc.player_id = 1;
 *    dc.target_acceleration = 2.0;
 *    ...
 *    tc.add_driver_control(dc);
 *    ...
 *    tc.add_trigger(cloe::Milliseconds(20));
 *    tc.send_packages()
 *
 * Of course, all these statements will be sprinkled around your code.
 */
class TaskControl : public VtdOmniSensor {
 public:
  explicit TaskControl(std::unique_ptr<RdbTransceiver>&& rdb_transceiver)
      : VtdOmniSensor(std::move(rdb_transceiver), UNDEFINED_OWNER_ID) {
    handler_.initMsg();
  }

  virtual ~TaskControl() = default;

  using VtdOmniSensor::process;
  void process(RDB_DRIVER_CTRL_t* driver_ctrl) override {
    // Steering speed at the front wheels [rad/s].
    if (driver_ctrl->validityFlags & RDB_DRIVER_INPUT_VALIDITY_STEERING_SPEED) {
      steering_wheel_speed_[driver_ctrl->playerId] = driver_ctrl->steeringSpeed;
    } else {
      vtd_logger()->warn("{}: steeringSpeed missing in RDB_DRIVER_CTRL_t", this->get_name());
      steering_wheel_speed_[driver_ctrl->playerId] = 0.0;
    }

    // Longitudinal acceleration request [m/s2].
    if (driver_ctrl->validityFlags & RDB_DRIVER_INPUT_VALIDITY_TGT_ACCEL) {
      driver_request_accel_[driver_ctrl->playerId] = driver_ctrl->accelTgt;
    } else {
      vtd_logger()->warn("{}: accelTgt missing in RDB_DRIVER_CTRL_t", this->get_name());
      driver_request_accel_[driver_ctrl->playerId] = 0.0;
    }
    // Steering request (angle at wheels) [rad].
    if (driver_ctrl->validityFlags & RDB_DRIVER_INPUT_VALIDITY_TGT_STEERING) {
      driver_request_steering_angle_[driver_ctrl->playerId] = driver_ctrl->steeringTgt;
    } else {
      vtd_logger()->warn("{}: steeringTgt missing in RDB_DRIVER_CTRL_t", this->get_name());
      driver_request_steering_angle_[driver_ctrl->playerId] = 0.0;
    }
  }

  void reset() override {
    VtdOmniSensor::reset();
    steering_wheel_speed_.clear();
    driver_request_accel_.clear();
    driver_request_steering_angle_.clear();
  }

  /**
   * Add driver control to the RDB message for the current frame.
   */
  void add_driver_control(const DriverControl& dc) {
    RDB_DRIVER_CTRL_t* driverCtrl =
        reinterpret_cast<RDB_DRIVER_CTRL_t*>(handler_.addPackage(0.0, 0, RDB_PKG_ID_DRIVER_CTRL));
    if (driverCtrl == nullptr) {
      vtd_logger()->error("TaskControl: cannot add RDB_PKG_ID_DRIVER_CTRL package");
      return;
    }

    driverCtrl->playerId = dc.player_id;
    driverCtrl->accelTgt = dc.target_acceleration;
    driverCtrl->steeringTgt = dc.target_steering;
    driverCtrl->flags = dc.driver_flags;
    driverCtrl->validityFlags = dc.validity_flags;
  }

  void add_dyn_object_state(const DynObjectState& os) {
    // TODO(tobias): actual sim time and frame no. needed?
    handler_.addPackage(0.0, 0, RDB_PKG_ID_START_OF_FRAME);
    RDB_OBJECT_STATE_t* objState = reinterpret_cast<RDB_OBJECT_STATE_t*>(
        handler_.addPackage(0.0, 0, RDB_PKG_ID_OBJECT_STATE, /*noElements=*/1,
                            /*extended=*/true));
    if (objState == nullptr) {
      vtd_logger()->error("TaskControl: cannot add RDB_OBJECT_STATE package");
      return;
    }
    objState->base.id = os.base_id;
    objState->base.category = os.base_category;
    objState->base.type = os.base_type;
    objState->base.visMask = os.base_vis_mask;
    std::strcpy(objState->base.name, os.base_name.c_str());
    objState->base.geo = os.base_geo;
    objState->base.pos = os.base_pos;
    objState->ext.speed = os.ext_speed;
    objState->ext.accel = os.ext_accel;
    handler_.addPackage(0.0, 0, RDB_PKG_ID_END_OF_FRAME);
  }

  /**
   * Add the trigger package, which specifies how much VTD should step.
   */
  void add_trigger(cloe::Duration delta_t) {
    RDB_TRIGGER_t* trigger =
        reinterpret_cast<RDB_TRIGGER_t*>(handler_.addPackage(0.0, 0, RDB_PKG_ID_TRIGGER));
    if (trigger == nullptr) {
      vtd_logger()->error("TaskControl: cannot add RDB_PKG_ID_DRIVER_CTRL package");
      return;
    }

    vtd_logger()->trace("TaskControl: setting trigger={} ns", delta_t.count());
    trigger->deltaT = std::chrono::duration_cast<std::chrono::duration<float>>(delta_t).count();
    trigger->frameNo = 0;
    trigger->features = 0;
  }

  /**
   * Send the packed RDB message to the task control server.
   */
  void send_packages() {
    rdb_->send(handler_.getMsg(), handler_.getMsgTotalSize());
    handler_.initMsg();
  }

  /**
   * Add the given trigger and then send it.
   *
   * A common idiom is to set the trigger and then right away send the
   * package to VTD. This will also send any other packages.
   */
  void add_trigger_and_send(cloe::Duration delta_t) {
    this->add_trigger(delta_t);
    this->send_packages();
  }

  /**
   * Get steering speed at front wheels of vehicle with given id [rad/s].
   */
  double get_steering_wheel_speed(uint64_t id) const { return steering_wheel_speed_.at(id); }

  /**
   * Get driver-requested longitudinal acceleration of vehicle with given id [m/s2].
   */
  double get_driver_request_acceleration(uint64_t id) const { return driver_request_accel_.at(id); }

  /**
   * Get driver-requested steering angle (at wheels) of vehicle with given id [rad].
   */
  double get_driver_request_steering_angle(uint64_t id) const {
    return driver_request_steering_angle_.at(id);
  }

  friend void to_json(cloe::Json& j, const TaskControl& tc) {
    j = cloe::Json{{"rdb_connection", tc.rdb_}};
  }

 protected:
  /// RDBHandler helps us conveniently construct RDB messages.
  Framework::RDBHandler handler_;
  std::map<int, double> steering_wheel_speed_;
  std::map<int, double> driver_request_accel_;
  std::map<int, double> driver_request_steering_angle_;
};

}  // namespace vtd
