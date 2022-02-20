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
 * \file actuator_component.hpp
 *
 * This file defines an implementation of LatLongActuator that sends it's
 * information to VTD.
 */

#pragma once

#include <memory>  // for shared_ptr<>

#include <cloe/component/latlong_actuator.hpp>  // for LatLongActuator

#include "task_control.hpp"  // for TaskControl
#include "vtd_logger.hpp"    // for vtd_logger

namespace vtd {

/**
 * VtdLatLongActuator implements LatLongActuator for the VTD binding.
 *
 * # Usage
 *
 * Every VTD cycle, the following needs to be done:
 *
 * - `has_level_change` must be used before `clear_cache` is called
 * - `add_actuation` registers any actuation with the TaskControl client,
 *   and must be called before `clear_cache`.
 * - `clear_cache` must be called before the cycle is over.
 * - `TaskControl::add_trigger_and_send` must be called to send the information
 *   to VTD.
 */
class VtdLatLongActuator : public cloe::LatLongActuator {
 public:
  VtdLatLongActuator(std::shared_ptr<TaskControl> tc, uint64_t id, const std::string& veh_name)
      : LatLongActuator("vtd/lat_long_actuator")
      , task_control_(tc)
      , vehicle_id_(id)
      , vehicle_name_(veh_name) {}
  virtual ~VtdLatLongActuator() = default;

  /**
   * Returns true when the controller actuation state changes from its previous
   * configuration.
   *
   * This should only be called after all controllers have run for a particular
   * simulation step. Unless of course you are interested if "so far" the state
   * is different or not. The "old state" with which the current state is
   * compared is the state that is present at the time that a control message
   * is sent to VTD. This means that after calling `send_driver_control`,
   * this method will definitely return false.
   */
  bool has_level_change() { return old_level_ != this->level_; }

  /**
   * Needs to be called after add_actuation and before the next
   * clear_cache invocation.
   */
  void save_level_state() { this->old_level_ = this->level_; }

  /**
   * Add the DriverControl package to the TaskControl.
   *
   * This should only be called once per simulation step. This method will not
   * pay attention for you.  Later, when the TaskControl sends its packages,
   * this one will be part of it.
   */
  void add_actuation() {
    if (is_vehicle_state()) {
      add_dyn_object_state();
    } else {
      add_driver_control();
    }
    // Detect driver or controller takeover for lateral and/or longitudinal control
    if (this->has_level_change()) {
      vtd_logger()->info("VtdLatLongActuator: vehicle {} controller state: {}", id(),
                         level_.to_human_cstr());
    }
  }

  void add_driver_control() {
    DriverControl dc;
    dc.player_id = vehicle_id_;

    if (is_acceleration()) {
      dc.target_acceleration = *acceleration();
      dc.validity_flags |= RDB_DRIVER_INPUT_VALIDITY_TGT_ACCEL;
    }

    if (is_steering_angle()) {
      dc.target_steering = *steering_angle();
      dc.validity_flags |= RDB_DRIVER_INPUT_VALIDITY_TGT_STEERING;
    }

    if (is_acceleration() || is_steering_angle()) {
      dc.validity_flags |= RDB_DRIVER_INPUT_VALIDITY_ADD_ON;
      task_control_->add_driver_control(dc);
    }
  }

  void add_dyn_object_state() {
    auto ego_state = vehicle_state();
    assert(ego_state);
    DynObjectState os;
    assert(static_cast<uint64_t>(ego_state->id) == vehicle_id_);
    os.base_id = ego_state->id;
    os.base_type = cloe_vtd_obj_class_map.at(ego_state->classification);
    os.base_name = vehicle_name_;
    os.base_geo = rdb_geometry_from_object(ego_state.get());
    os.base_pos = rdb_coord_from_object(ego_state.get());
    os.ext_speed = rdb_coord_from_vector3d(ego_state->velocity, ego_state->angular_velocity);
    os.ext_accel = rdb_coord_pos_from_vector3d(ego_state->acceleration);
    task_control_->add_dyn_object_state(os);
  }

  cloe::Duration process(const cloe::Sync& sync) override { return LatLongActuator::process(sync); }

  void reset() override {
    old_level_.set_none();
    LatLongActuator::reset();
    task_control_->reset();
  }

 private:
  std::shared_ptr<TaskControl> task_control_;
  uint64_t vehicle_id_;
  std::string vehicle_name_;
  cloe::utility::ActuationLevel old_level_;
};

}  // namespace vtd
