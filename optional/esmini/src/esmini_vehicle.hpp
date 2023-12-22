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
 * \file esmini_vehicle.hpp
 *
 * `ESMiniVehicle` is the implementation of a vehicle that is provided with
 *  ground truth data from the `ESMini` scenario player.
 *
 * \see cloe/vehicle.hpp
 *
 */

#include <float.h>  // for DBL_MAX

#include <cloe/component/lane_boundary.hpp>             // for LaneBoundary
#include <cloe/component/lane_sensor_functional.hpp>    // for LaneSensorFilter
#include <cloe/component/object_sensor_functional.hpp>  // for ObjectSensorFilter
#include "esmini_ego_control.hpp"                       // for ESMiniEgoControl
#include "esmini_osi_sensor.hpp"                        // for ESMiniOsiSensor
#include "esmini_sensor_components.hpp"  // for ESMiniEgoSensor, ESMiniObjectSensor, ..
#include "esmini_world_data.hpp"         // for ESMiniEnvData

namespace esmini {

class ESMiniVehicle : public cloe::Vehicle {
 public:
  /**
   * Construct a `ESMiniVehicle`.
   *
   * \arg id     unique ID within simulator's set of vehicles
   * \arg name   unique name within simulator's set of vehicles
   * \arg config vehicle configuration
   *
   */
  ESMiniVehicle(uint64_t id, const std::string& name, const ESMiniVehicleConfig& config)
      : Vehicle(id, name) {
    auto osi_gnd_truth = std::make_shared<ESMiniOsiSensor>(id, config.filter_dist);
    env_data_ = osi_gnd_truth;
    osi_gnd_truth->set_name(name + "_osi_sensor");

    // Add ego sensor
    this->new_component(new ESMiniEgoSensor{id, osi_gnd_truth},
                        cloe::CloeComponent::GROUNDTRUTH_EGO_SENSOR,
                        cloe::CloeComponent::DEFAULT_EGO_SENSOR);

    // Add object sensor
    this->new_component(new ESMiniObjectSensor{osi_gnd_truth},
                        cloe::CloeComponent::GROUNDTRUTH_WORLD_SENSOR,
                        cloe::CloeComponent::DEFAULT_WORLD_SENSOR);

    // Only keep object data within configured filter distance.
    auto filter_fct_obj = [fdist = config.filter_dist](const cloe::Object& obj) {
      // Object position is stored in sensor coordinate system.
      return obj.pose.translation().norm() < fdist;
    };
    this->emplace_component(
        std::make_shared<cloe::ObjectSensorFilter>(
            this->get<cloe::ObjectSensor>(cloe::CloeComponent::DEFAULT_WORLD_SENSOR),
            filter_fct_obj),
        cloe::CloeComponent::DEFAULT_WORLD_SENSOR);

    // Add lane-boundary sensor
    this->new_component(new ESMiniLaneBoundarySensor{osi_gnd_truth},
                        cloe::CloeComponent::GROUNDTRUTH_LANE_SENSOR,
                        cloe::CloeComponent::DEFAULT_LANE_SENSOR);

    // Only keep lane boundary data within configured filter distance.
    auto filter_fct_lbs = [fdist = config.filter_dist](const cloe::LaneBoundary& lb) {
      double min_dist = DBL_MAX;
      for (uint64_t i = 1; i < lb.points.size(); ++i) {
        const auto& pt0 = lb.points[i - 1];
        const auto& pt1 = lb.points[i];
        Eigen::Vector3d dpt = pt1 - pt0;
        // Compute minimum distance of the line through two neighboring points and the sensor frame origin.
        // See https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Vector_formulation.
        double dpt_abs = dpt.norm();
        if (dpt_abs > 0.1) {
          // Unit directional vector.
          dpt /= dpt_abs;
          // Compute point with minimum distance to origin (=sensor mounting position).
          Eigen::Vector3d ptm = pt0 - (pt0.dot(dpt)) * dpt;
          // Only use that point if it is located between pt0 and pt1.
          double s;
          if (dpt.x() > dpt.y()) {
            s = (ptm.x() - pt0.x()) / dpt.x();
          } else {
            s = (ptm.y() - pt0.y()) / dpt.y();
          }
          if (s <= 0) {
            min_dist = std::min(min_dist, pt0.norm());
          } else if (s >= dpt_abs) {
            min_dist = std::min(min_dist, pt1.norm());
          } else {
            min_dist = std::min(min_dist, ptm.norm());
          }
        } else {
          // Negligible distance between neighboring points.
          min_dist = std::min(min_dist, pt0.norm());
        }
      }
      return min_dist < fdist;
    };

    this->emplace_component(
        std::make_shared<cloe::LaneBoundarySensorFilter>(
            this->get<cloe::LaneBoundarySensor>(cloe::CloeComponent::DEFAULT_LANE_SENSOR),
            filter_fct_lbs),
        cloe::CloeComponent::DEFAULT_LANE_SENSOR);

    // TODO(tobias): Add groundtruth world sensor and emplace it by a static object only version

    if (config.is_closed_loop) {
      // Add actuator component to receive target acceleration and steering angle from controller.
      ego_control_ = std::make_shared<ESMiniEgoControl>(id);
      this->add_component(ego_control_,
                          cloe::CloeComponent::GROUNDTRUTH_LATLONG_ACTUATOR,
                          cloe::CloeComponent::DEFAULT_LATLONG_ACTUATOR);
    }
  }

  /**
   * Do everything that a vehicle needs before a simulator step is triggered.
   *
   * Update the ego vehicle position in the scene.
   */
  void esmini_step_ego_position(const cloe::Sync& s) {
    if (ego_control_) {
      this->ego_control_->step(s);
    }
  }

  /**
   * Do everything that a vehicle needs after a simulator step is triggered.
   */
  cloe::Duration esmini_get_environment_data(const cloe::Sync& s) {
    env_data_->step(s);
    return env_data_->time();
  }

  /**
   * The vehicle update functions are called from the simulator binding directly, so not much to do here.
   */
  cloe::Duration process(const cloe::Sync& sync) final { return Vehicle::process(sync); }

 private:
  /// Ground truth data in vehicle vicinity in ego vehicle reference frame.
  std::shared_ptr<ESMiniEnvData> env_data_;
  /// Vehicle position control.
  std::shared_ptr<ESMiniEgoControl> ego_control_{nullptr};
};

}  // namespace esmini
