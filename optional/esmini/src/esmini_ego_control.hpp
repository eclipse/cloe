/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file esmini_ego_control.hpp
 *
 * This file defines an implementation of LatLongActuator that updates the ego
 * vehicle state in ESMini from a simple vehicle and driver model.
 */

#pragma once

#include <algorithm>  // for min, max
#include <cmath>      // for fabs, remainder, atan2, M_PI_2
#include <memory>     // for shared_ptr<>

#include <esminiLib.hpp>  // for SE_SimpleVehicleControlAnalog, ..

#include <cloe/component/latlong_actuator.hpp>  // for LatLongActuator

#include "esmini_logger.hpp"  // for esmini_logger

// Refer to comment on Controller::Type in esminiLib.hpp.
#define ESMINI_CONTROLLER_TYPE_EXTERNAL 1

namespace esmini {

/**
 * ESMiniSimpleEgoModel updates the ego state based on a kinematic bicycle model.
 *
 * For details on the model, refer to https://ieeexplore.ieee.org/document/7225830.
 */
class ESMiniSimpleEgoModel {
 public:
  ESMiniSimpleEgoModel(uint64_t ego_id) { init_model(ego_id); }
  virtual ~ESMiniSimpleEgoModel() = default;

  void step(double dt_sec, double trg_throttle, double trg_front_wheel_angle) {
    // Different from the documentation, `steerAngle` seems to be the target wheel angle in radians:
    // https://github.com/esmini/esmini/blob/master/EnvironmentSimulator/Modules/Controllers/vehicle.cpp#L172
    SE_SimpleVehicleControlAnalog(model_, dt_sec, trg_throttle, trg_front_wheel_angle);
  }

  const SE_SimpleVehicleState& get_ego_state() {
    SE_SimpleVehicleGetState(model_, &ego_state_);
    return ego_state_;
  }

  double get_throttle_from_acceleration(double trg_accel) const {
    return std::max(-1.0, std::min(1.0, trg_accel / max_acceleration_abs_));
  }

 private:
  void init_model(uint64_t ego_id) {
    // Retrieve the ego state from the scenario.
    SE_ScenarioObjectState sc_ego_state;
    SE_GetObjectState(ego_id, &sc_ego_state);
    if (sc_ego_state.ctrl_type != ESMINI_CONTROLLER_TYPE_EXTERNAL) {
      throw cloe::ModelError(
          "ESMiniSimpleEgoModel: esminiController must be set to ExternalController in .xosc "
          "file.");
    }
    // Instantiate the vehicle model.
    if (sc_ego_state.length <= 0.0) {
      throw cloe::ModelError("ESMiniSimpleEgoModel: Unphysical ego length received.");
    }
    model_ = SE_SimpleVehicleCreate(sc_ego_state.x, sc_ego_state.y, sc_ego_state.h,
                                    sc_ego_state.length, sc_ego_state.speed);
    // Configure the vehicle model.
    SE_SimpleVehicleSetMaxAcceleration(model_, max_acceleration_abs_);
    SE_SimpleVehicleSetMaxDeceleration(model_, max_acceleration_abs_);
    SE_SimpleVehicleSetEngineBrakeFactor(model_, engine_brake_factor_);
    SE_SimpleVehicleSteeringRate(model_, steering_rate_);
    SE_SimpleVehicleSteeringReturnFactor(model_, steering_return_factor_);
    // Set thresholds used for clipping the model results.
    SE_SimpleVehicleSetMaxSpeed(model_, max_speed_kph_);
    SE_SimpleVehicleSteeringScale(model_, steering_scale_);
  }

 private:
  /// Maximum longitudinal acceleration magnitude (ESMini default: 20 m/s^2). Stay below virtue limit of 20 m/s^2.
  const double max_acceleration_abs_{19};
  /// Steering rate for lateral control (ESMini default: 8 1/s).
  const double steering_rate_{8};
  /// Engine brake factor (ESMini default: 0.001).
  const double engine_brake_factor_{0.001};
  /// Wheel return factor (ESMini default: 4.0).
  const double steering_return_factor_{4.0};
  /// Clip unphysical velocities (ESMini default: 70 m/s).
  const double max_speed_kph_{300.0};
  /// Clip steering angle speed-dependent (ESMini default: 0.02).
  const double steering_scale_{0.02};
  /// Handle to the model instance.
  void* model_{nullptr};
  /// Ego vehicle state.
  SE_SimpleVehicleState ego_state_{0, 0, 0, 0, 0, 0, 0, 0};
};

enum class DriverModelType {
  Simple,  ///< Look ahead to a point on the current lane and steer towards it.
  GhostLookAheadTime,  ///< Use ghost vehicle state in some time ahead to obtain target acceleration and steering.
  GhostLookAheadDist,  ///< Use ghost vehicle state in some distance ahead to obtain target acceleration and steering.
};

// clang-format off
ENUM_SERIALIZATION(DriverModelType, ({
  {DriverModelType::Simple, "simple"},
  {DriverModelType::GhostLookAheadTime, "ghost_time"},
  {DriverModelType::GhostLookAheadDist, "ghost_distance"},
}))
// clang-format on

class ESMiniDriverModel {
 public:
  ESMiniDriverModel(uint64_t ego_id, DriverModelType type) : ego_id_(ego_id), model_type_(type) {}
  virtual ~ESMiniDriverModel() = default;

  void step(double ego_vel, double& throttle, double& steering_angle) {
    throttle = 0.0;
    steering_angle = 0.0;
    SE_RoadInfo road_info;
    float target_vel;
    esmini_logger()->info("ESMiniDriverModel at {}s", SE_GetSimulationTime());
    // Check if ghost vehicle is driving ahead of ego.
    if (SE_ObjectHasGhost(ego_id_) == 1) {
      SE_ScenarioObjectState ego_state, ghost_state;
      SE_GetObjectState(ego_id_, &ego_state);
      SE_GetObjectGhostState(ego_id_, &ghost_state);
      double ghost_dir_angle = atan2(ghost_state.y - ego_state.y, ghost_state.x - ego_state.x);
      double delta_ego_dir = remainder(ghost_dir_angle - ego_state.h, 2 * M_PI);
      if (abs(delta_ego_dir) > M_PI_2) {
        throw cloe::ModelError(
            "ESMiniDriverModel: Ego vehicle has passed driver model ghost object. Fix scenario.");
      }
    } else {
      throw cloe::ModelError(
          "ESMiniDriverModel: Ghost vehicle missing. Refer to test-driver.xosc for an example how "
          "to set property \"useGhost\".");
    }
    // Determine the target velocity and steering angle at the look-ahead point.
    if (model_type_ == DriverModelType::GhostLookAheadDist) {
      SE_GetRoadInfoAlongGhostTrail(ego_id_, 5 + 0.75f * ego_vel, &road_info, &target_vel);
    } else if (model_type_ == DriverModelType::GhostLookAheadTime) {
      SE_GetRoadInfoGhostTrailTime(ego_id_, SE_GetSimulationTime() + 0.25f, &road_info,
                                   &target_vel);
    } else {
      // Use simple model (refer to the ESMini test-driver.cpp example).
      // Look ahead along lane center. Scenario actions are ignored.
      uint16_t lookahead_strategy = 0;
      SE_GetRoadInfoAtDistance(ego_id_, 5 + 0.75f * ego_vel, &road_info, lookahead_strategy, true);
      if (road_info.speed_limit <= 0.0) {
        throw cloe::ModelError("ESMiniDriverModel::Simple: OpenDrive speed limit missing.");
      }
      // Slow down in curves using tuning parameter.
      double curve_weight = 30.0;
      target_vel = road_info.speed_limit / (1.0 + curve_weight * std::fabs(road_info.angle));
    }
    throttle = throttle_weight_ * (target_vel - ego_vel);
    throttle = std::max(-1.0, std::min(1.0, throttle));
    steering_angle = road_info.angle;
  }

 private:
  /// ID of the ESMini ego object.
  uint64_t ego_id_;
  /// ESMini driver model according to test-driver.cpp example.
  DriverModelType model_type_{DriverModelType::GhostLookAheadDist};
  /// Tuning parameter to reach the target velocity.
  const double throttle_weight_{0.1};
  /// Handle to the model instance.
  void* model_{nullptr};
};

/**
 * ESMiniEgoControl implements LatLongActuator for the ESMini binding. The ego vehicle position is updated in the
 * ESMini scene using a simple vehicle model. The new ego state is either computed from the control request or a simple
 * driver model.
 */
class ESMiniEgoControl : public cloe::LatLongActuator {
 public:
  ESMiniEgoControl(uint64_t id) : LatLongActuator("esmini/lat_long_actuator"), ego_id_(id) {
    vehicle_model_ = std::make_shared<ESMiniSimpleEgoModel>(ego_id_);
    driver_model_ =
        std::make_shared<ESMiniDriverModel>(ego_id_, DriverModelType::GhostLookAheadDist);
  }
  virtual ~ESMiniEgoControl() = default;

  /**
   * Returns true when the controller actuation state changes from its previous
   * configuration (lateral, longitudinal control, both or none).
   */
  bool has_level_change() { return old_level_ != this->level_; }

  /**
   * Needs to be called after add_driver_control and before the next
   * clear_cache invocation.
   */
  void save_level_state() { this->old_level_ = this->level_; }

  /**
   * Update the ego vehicle position in the scene.
   */
  void step(const cloe::Sync& s) {
    double trg_throttle;
    double trg_steering_angle;
    get_target_actuation(trg_throttle, trg_steering_angle);
    // Step the vehicle model forward in time.
    vehicle_model_->step(std::chrono::duration<double>(s.step_width()).count(), trg_throttle,
                         trg_steering_angle);
    // Get the updated vehicle state.
    auto ego_state = vehicle_model_->get_ego_state();
    // Update new ego position, heading and velocity in the scenario (z, pitch, roll will be aligned to the road).
    SE_ReportObjectPosXYH(0, 0, ego_state.x, ego_state.y, ego_state.h, ego_state.speed);
    // Detect driver or controller takeover for lateral and/or longitudinal control.
    if (this->has_level_change()) {
      esmini_logger()->info("ESMiniEgoControl: vehicle {} new controller state: {}", id(),
                            level_.to_human_cstr());
    }
    save_level_state();
  }

  cloe::Duration process(const cloe::Sync& sync) override { return LatLongActuator::process(sync); }

  void reset() override {
    old_level_.set_none();
    LatLongActuator::reset();
  }

 private:
  void get_target_actuation(double& trg_throttle, double& trg_angle) const {
    if (!target_acceleration_ || !target_steering_angle_) {
      // Use driver model to obtain target actuation values.
      auto ego_state = vehicle_model_->get_ego_state();
      driver_model_->step(ego_state.speed, trg_throttle, trg_angle);
    }
    // Use actuation values provided by a controller, if available.
    if (target_acceleration_) {
      trg_throttle = vehicle_model_->get_throttle_from_acceleration(*target_acceleration_);
    }
    if (target_steering_angle_) {
      trg_angle = *target_steering_angle_;
    }
  }

 private:
  uint64_t ego_id_;
  cloe::utility::ActuationLevel old_level_;
  std::shared_ptr<ESMiniSimpleEgoModel> vehicle_model_{nullptr};
  std::shared_ptr<ESMiniDriverModel> driver_model_{nullptr};
};

}  // namespace esmini
