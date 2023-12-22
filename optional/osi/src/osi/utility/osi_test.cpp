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
 * \file osi_test.cpp
 * \see osi_omni_sensor.hpp
 * \see osi_omni_sensor.cpp
 * \see osi_utils.hpp
 * \see osi_utils.cpp
 */

#include <cmath>  // for M_PI, M_PI_2

#include <gtest/gtest.h>   // for TEST, ASSERT_TRUE
#include <Eigen/Geometry>  // for Isometry3d, Vector3d

#include <cloe/component/object.hpp>  // for Object
#include <cloe/utility/geometry.hpp>  // for quaternion_from_rpy

#include "osi_common.pb.h"  // for Orientation3D, BaseMoving, ..
#include "osi_object.pb.h"  // for MovingObject

#include "osi/utility/osi_omni_sensor.hpp"  // for transform_ego_coord_from_osi_data, ...
#include "osi/utility/osi_utils.hpp"        // for pose_to_osi_position_orientation, ...

// Sensor mounting position in the vehicle reference frame
constexpr double sens_pos_xyz[] = {3.0, 1.0, 0.0};
constexpr double sens_ori_rpy[] = {0.0, 0.0, M_PI_2};

void init_osi_vec_3d(osi3::Vector3d* v, const double d[3]) {
  v->set_x(d[0]);
  v->set_y(d[1]);
  v->set_z(d[2]);
}

void init_osi_ori_3d(osi3::Orientation3d* o, const double d[3]) {
  o->set_roll(d[0]);
  o->set_pitch(d[1]);
  o->set_yaw(d[2]);
}

void assert_eq_osi_vec_3d(const osi3::Vector3d& v, const osi3::Vector3d& w) {
  ASSERT_DOUBLE_EQ(v.x(), w.x());
  ASSERT_DOUBLE_EQ(v.y(), w.y());
  ASSERT_DOUBLE_EQ(v.z(), w.z());
}

void assert_eq_osi_ori_3d(const osi3::Orientation3d& o, const osi3::Orientation3d& p) {
  ASSERT_DOUBLE_EQ(o.roll(), p.roll());
  ASSERT_DOUBLE_EQ(o.pitch(), p.pitch());
  ASSERT_DOUBLE_EQ(o.yaw(), p.yaw());
}

TEST(osi, eigen_pose) {
  // Test conversions from OSI to Eigen data format.
  osi3::BaseMoving base;
  init_osi_vec_3d(base.mutable_position(), sens_pos_xyz);
  constexpr double ori[3] = {0.1 * M_PI, 0.2 * M_PI, 0.3 * M_PI};
  init_osi_ori_3d(base.mutable_orientation(), ori);
  Eigen::Isometry3d pose = osii::osi_position_orientation_to_pose(base);
  // Inverse conversion.
  osi3::BaseMoving base_out;
  osii::pose_to_osi_position_orientation(pose, base_out);

  assert_eq_osi_vec_3d(base.position(), base_out.position());
  assert_eq_osi_ori_3d(base.orientation(), base_out.orientation());
}

TEST(osi, transf_base_mov) {
  // Test coordinate transformation from global to ego reference frame.
  struct ObjectData {
    double pos[3];
    double rpy[3];
    double vel[3];
    double acc[3];
    double rpy_dot[3];
  };
  // Define ego attributes (global coordinates).
  constexpr ObjectData ego_data = {
      {0.0, -10.0, 0.0},       // pos
      {0.0, 0.0, -M_PI_2},     // rpy
      {0.0, -20.0, 0.0},       // vel
      {0.0, -1.0, 0.0},        // acc
      {0.0, 0.0, 0.1 * M_PI},  // rpy_dot
  };
  // Define target attributes (global coordinates).
  constexpr ObjectData obj_data = {
      {10.0, -20.0, 0.0},      // pos
      {0.0, 0.0, -M_PI_2},     // rpy
      {10.0, -30.0, 0.0},      // vel
      {0.0, -2.0, 0.0},        // acc
      {0.0, 0.0, 0.1 * M_PI},  // rpy_dot
  };

  auto init_osi_base = [](const ObjectData& o) {
    osi3::BaseMoving b;
    init_osi_vec_3d(b.mutable_position(), o.pos);
    init_osi_ori_3d(b.mutable_orientation(), o.rpy);
    init_osi_vec_3d(b.mutable_velocity(), o.vel);
    init_osi_vec_3d(b.mutable_acceleration(), o.acc);
    init_osi_ori_3d(b.mutable_orientation_rate(), o.rpy_dot);
    return b;
  };
  // Set the ego OSI data.
  osi3::BaseMoving ego_base = init_osi_base(ego_data);
  // Set the target object OSI data.
  osi3::BaseMoving obj_base = init_osi_base(obj_data);
  // Transform the object base into the ego reference frame.
  osii::osi_transform_base_moving(ego_base, obj_base);

  ASSERT_DOUBLE_EQ(obj_base.position().x(), 10.0);
  ASSERT_DOUBLE_EQ(obj_base.position().y(), 10.0);
  ASSERT_DOUBLE_EQ(obj_base.position().z(), 0.0);

  ASSERT_DOUBLE_EQ(obj_base.orientation().roll(), 0.0);
  ASSERT_DOUBLE_EQ(obj_base.orientation().pitch(), 0.0);
  ASSERT_DOUBLE_EQ(obj_base.orientation().yaw(), 0.0);

  ASSERT_DOUBLE_EQ(obj_base.velocity().x(), 10.0);
  ASSERT_DOUBLE_EQ(obj_base.velocity().y(), 10.0);
  ASSERT_DOUBLE_EQ(obj_base.velocity().z(), 0.0);

  ASSERT_DOUBLE_EQ(obj_base.acceleration().x(), 1.0);

  ASSERT_DOUBLE_EQ(obj_base.orientation_rate().yaw(), 0.0);
}

// Ego object attributes for coordinate transformation tests.
const Eigen::Vector3d obj_dims{3.0, 2.0, 1.8};
const Eigen::Vector3d obj_pos{10.0, 10.0, 10.0};
const Eigen::Vector3d obj_vel{0.0, 10.0, 0};
const Eigen::Vector3d obj_osi_cog{-1.2, 0.0, -0.5};
constexpr double obj_rpy[] = {sens_ori_rpy[0], sens_ori_rpy[1], sens_ori_rpy[2]};

TEST(osi, transform_ego_coord) {
  // Test the transformation of the ego vehicle reference point to Cloe format.
  cloe::Object ego;

  // The ego vehicle is driving in positive y-direction.
  auto quat = cloe::utility::quaternion_from_rpy(obj_rpy[0], obj_rpy[1], obj_rpy[2]);
  ego.pose = cloe::utility::pose_from_rotation_translation(quat, obj_pos);
  ego.velocity = obj_vel;
  ego.dimensions = obj_dims;
  // OSI bbcenter_to_rear in local object reference frame.
  ego.cog_offset = obj_osi_cog;

  osii::transform_ego_coord_from_osi_data(obj_dims, ego);

  // Result: Ego rear axle center on street level, in world frame.
  ASSERT_DOUBLE_EQ(ego.pose.translation()(0), 10.0);
  ASSERT_DOUBLE_EQ(ego.pose.translation()(1), 8.8);
  ASSERT_DOUBLE_EQ(ego.pose.translation()(2), 9.1);

  ASSERT_DOUBLE_EQ(ego.cog_offset(0), -obj_osi_cog(0));
  ASSERT_DOUBLE_EQ(ego.cog_offset(1), 0.0);
  ASSERT_DOUBLE_EQ(ego.cog_offset(2), 0.0);

  // Velocity in ego vehicle frame.
  ASSERT_DOUBLE_EQ(ego.velocity(0), obj_vel(1));
}

TEST(osi, transform_obj_coord) {
  // Test the transformation from ego vehicle frame into sensor frame.
  cloe::Object obj;

  // The target object is driving in positive y-direction.
  auto quat = cloe::utility::quaternion_from_rpy(obj_rpy[0], obj_rpy[1], obj_rpy[2]);
  obj.pose = cloe::utility::pose_from_rotation_translation(quat, obj_pos);
  obj.velocity = obj_vel;
  Eigen::Vector3d obj_ang_vel{0.0, 1.0, 0};
  obj.angular_velocity = obj_ang_vel;
  obj.dimensions = obj_dims;
  // OSI bbcenter_to_rear in local object frame.
  obj.cog_offset = obj_osi_cog;

  /// Set sensor pose relative to the ego frame (rear axle center).
  quat = cloe::utility::quaternion_from_rpy(sens_ori_rpy[0], sens_ori_rpy[1], sens_ori_rpy[2]);
  Eigen::Vector3d transl{sens_pos_xyz[0], sens_pos_xyz[1], sens_pos_xyz[2]};
  Eigen::Isometry3d sensor_pose = cloe::utility::pose_from_rotation_translation(quat, transl);

  osii::transform_obj_coord_from_osi_data(sensor_pose, obj_dims, obj);

  // Result: Object rear axle center on street level, in sensor frame.
  ASSERT_DOUBLE_EQ(obj.pose.translation()(0), 7.8);
  ASSERT_DOUBLE_EQ(obj.pose.translation()(1), -7.0);
  ASSERT_DOUBLE_EQ(obj.pose.translation()(2), 9.1);

  ASSERT_DOUBLE_EQ(obj.cog_offset(0), -obj_osi_cog(0));
  ASSERT_DOUBLE_EQ(obj.cog_offset(1), 0.0);
  ASSERT_DOUBLE_EQ(obj.cog_offset(2), 0.0);

  // Velocity and acceleration in sensor reference frame.
  ASSERT_DOUBLE_EQ(obj.velocity(0), obj_vel(1));
  ASSERT_DOUBLE_EQ(obj.angular_velocity(0), obj_ang_vel(1));
}

TEST(osi, vehicle_classification) {
  // Test vehicle type/classification conversion from OSI to Cloe.
  osi3::MovingObject osi_obj;
  osi3::MovingObject_VehicleClassification* osi_class =
      new osi3::MovingObject_VehicleClassification;

  osi_obj.set_type(osi3::MovingObject_Type_TYPE_VEHICLE);
  osi_obj.set_allocated_vehicle_classification(osi_class);
  ASSERT_TRUE(osi_obj.has_vehicle_classification());
  osi_obj.mutable_vehicle_classification()->set_type(
      osi3::MovingObject_VehicleClassification_Type_TYPE_SMALL_CAR);

  cloe::Object obj;

  osii::from_osi_mov_obj_type_classification(osi_obj, obj.classification);

  ASSERT_EQ(obj.classification, cloe::Object::Class::Car);
}
