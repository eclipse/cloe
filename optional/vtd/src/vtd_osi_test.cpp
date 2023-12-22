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
 * \file vtd_osi_test.cpp
 * \see  osi_sensor_component.hpp
 * \see  osi_message_handler.hpp
 * \see  osi_message_handler.cpp
 */

#include <map>

#include <gtest/gtest.h>              // for TEST, ASSERT_TRUE
#include <cloe/component/object.hpp>  // for Object
#include <cloe/core/duration.hpp>     // for Duration
#include <cloe/utility/geometry.hpp>  // for quaternion_from_rpy

#include "osi_common.pb.h"          // for Timestamp, Identifier, BaseMoving, ..
#include "osi_detectedobject.pb.h"  // for DetectedMovingObject
#include "osi_groundtruth.pb.h"     // for GroundTruth
#include "osi_object.pb.h"          // for MovingObject
#include "osi_sensordata.pb.h"      // for SensorData, DetectedEntityHeader
#include "osi_sensorview.pb.h"      // for SensorView

#include <osi/utility/osi_message_handler.hpp>
#include <osi/utility/osi_transceiver_tcp.hpp>

#include "osi_sensor_component.hpp"  // for transform_...

struct VehicleData {
  uint64_t id;
  osi3::MovingObject_Type vt;
  osi3::MovingObject_VehicleClassification_Type vc;
};

// Initialize the optional fields required by the Cloe/OSI interface.
void init_osi_base(osi3::BaseMoving* b) {
  b->mutable_position();
  b->mutable_orientation();
  b->mutable_dimension();
  b->mutable_velocity();
  b->mutable_acceleration();
  b->mutable_orientation_rate();
}

void init_osi_ground_truth(osi3::GroundTruth* gt, const std::map<std::string, VehicleData>& veh) {
  gt->mutable_host_vehicle_id()->set_value(veh.at("ego").id);
  // Add all players.
  for (int i = 0; i < veh.size(); ++i) {
    gt->add_moving_object();
  }
  for (auto& kv : veh) {
    auto& v = kv.second;
    osi3::MovingObject* gt_obj;
    gt_obj = gt->mutable_moving_object(v.id);
    gt_obj->mutable_id()->set_value(v.id);
    init_osi_base(gt_obj->mutable_base());
    gt_obj->set_type(v.vt);
    gt_obj->mutable_vehicle_classification()->set_type(v.vc);
    gt_obj->mutable_vehicle_attributes()->mutable_bbcenter_to_rear();
  }
}

void init_osi_detected_objects(osi3::SensorData* data,
                               const std::map<std::string, VehicleData>& veh) {
  for (auto& kv : veh) {
    if (kv.first == "ego") {
      continue;
    }
    auto& v = kv.second;
    osi3::DetectedMovingObject* osi_obj;
    osi_obj = data->add_moving_object();
    osi_obj->mutable_header()->add_ground_truth_id()->set_value(v.id);
    init_osi_base(osi_obj->mutable_base());
    osi_obj->add_candidate();
    osi3::DetectedMovingObject::CandidateMovingObject* cand_obj;
    cand_obj = osi_obj->mutable_candidate(0);
    // Copy object info from ground truth.
    osi3::MovingObject gt_obj;
    gt_obj = data->sensor_view(0).global_ground_truth().moving_object(v.id);
    cand_obj->set_type(gt_obj.type());
    cand_obj->mutable_vehicle_classification()->CopyFrom(gt_obj.vehicle_classification());
  }
}

TEST(vtd_osi, osi_sensor) {
  // If the required OSI data fields are not initialized, the test will fail.
  const std::map<std::string, VehicleData> vehicles = {
      {"ego",
       {1, osi3::MovingObject_Type_TYPE_VEHICLE,
        osi3::MovingObject_VehicleClassification_Type_TYPE_MEDIUM_CAR}},
      {"target",
       {0, osi3::MovingObject_Type_TYPE_VEHICLE,
        osi3::MovingObject_VehicleClassification_Type_TYPE_SMALL_CAR}}};
  vtd::VtdOsiSensor sensor(std::unique_ptr<cloeosi::OsiTransceiver>(nullptr), vehicles.at("ego").id);
  cloe::Duration sim_time{0};
  std::shared_ptr<cloeosi::SensorMockConf> mock_conf = std::make_shared<cloeosi::SensorMockConf>();
  sensor.set_mock_conf(mock_conf);
  // Initialize sensor data.
  auto data_shp = std::make_shared<osi3::SensorData>();
  osi3::SensorData* data = data_shp.get();
  data->mutable_version()->set_version_major(3);
  data->mutable_timestamp()->set_seconds(1);
  data->mutable_last_measurement_time()->set_seconds(0);
  data->mutable_mounting_position()->mutable_position();
  data->mutable_mounting_position()->mutable_orientation();
  // Initialize sensor view.
  osi3::SensorView* view;
  view = data->add_sensor_view();
  // Initialize ground truth.
  init_osi_ground_truth(view->mutable_global_ground_truth(), vehicles);
  // Initialize detected objects.
  init_osi_detected_objects(data, vehicles);

  ASSERT_GT(data->ByteSizeLong(), 0);
  ASSERT_NO_THROW(sensor.process_received_msg(data, sim_time));
}
