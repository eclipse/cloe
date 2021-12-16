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

#include <cloe/core.hpp>  // for Json

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
