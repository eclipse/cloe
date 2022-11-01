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

class VtdVehicleControl {
 public:
  VtdVehicleControl() = default;
  virtual ~VtdVehicleControl() = default;

  /**
   * Add the DriverControl or DynObjectState package to the TaskControl.
   *
   * This should only be called once per simulation step. This method will not
   * pay attention for you.  Later, when the TaskControl sends its packages,
   * this one will be part of it.
   */
  virtual void step_begin(const cloe::Sync& sync) = 0;

  /**
   * Operations after vehicle control information was added to the TaskControl
   * message and the vehicle labels were set.
   */
  virtual void step_end(const cloe::Sync&){};

  /**
   * Return true, if the label text should be updated.
   */
  virtual bool update_vehicle_label() { return false; }

  /**
   * Return the current actuation level, if applicable.
   */
  virtual cloe::utility::ActuationLevel get_actuation_level() {
    return cloe::utility::ActuationLevel::None;
  }

  virtual void reset(){};

  virtual cloe::Json to_json() const = 0;

  friend void to_json(cloe::Json& j, const VtdVehicleControl& vc) { j = vc.to_json(); }
};

/**
 * VtdLatLongActuator implements LatLongActuator for the VTD binding.
 *
 * # Usage
 *
 * Every VTD cycle, the following needs to be done:
 *
 * - `has_level_change` must be used before `clear_cache` is called
 * - `step_begin` registers any actuation with the TaskControl client,
 *   and must be called before `clear_cache`.
 * - `clear_cache` must be called before the cycle is over.
 * - `TaskControl::add_trigger_and_send` must be called to send the information
 *   to VTD.
 */
class VtdLatLongActuator : public VtdVehicleControl, public cloe::LatLongActuator {
 public:
  VtdLatLongActuator(std::shared_ptr<TaskControl> tc, uint64_t id)
      : LatLongActuator("vtd/lat_long_actuator"), task_control_(tc), vehicle_id_(id) {}
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
  bool update_vehicle_label() override { return this->has_level_change(); }

  cloe::utility::ActuationLevel get_actuation_level() override { return this->actuation_level(); }

  cloe::Duration process(const cloe::Sync& sync) override { return LatLongActuator::process(sync); }

  void reset() override {
    old_level_.set_none();
    LatLongActuator::reset();
    task_control_->reset();
  }

  void step_begin(const cloe::Sync&) override {
    add_driver_control();
    // Detect driver or controller takeover for lateral and/or longitudinal control
    if (this->has_level_change()) {
      vtd_logger()->info("VtdLatLongActuator: vehicle {} controller state: {}", id(),
                         level_.to_human_cstr());
    }
  }

  void step_end(const cloe::Sync&) override { this->old_level_ = this->level_; }

  cloe::Json to_json() const override { return dynamic_cast<const cloe::LatLongActuator&>(*this); }

 private:
  bool has_level_change() { return old_level_ != this->level_; }

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

 private:
  std::shared_ptr<TaskControl> task_control_;
  uint64_t vehicle_id_;
  cloe::utility::ActuationLevel old_level_;
};

    }
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
  cloe::utility::ActuationLevel old_level_;
};

}  // namespace vtd
