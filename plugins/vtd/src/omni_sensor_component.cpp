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
 * \file omni_sensor_component.cpp
 * \see  omni_sensor_component.hpp
 */

#include "omni_sensor_component.hpp"

#include <map>     // for map<>
#include <string>  // for string

#include <Eigen/Geometry>  // for Vector3d, Quaterniond

#include <viRDBIcd.h>  // for RDB_OBJECT_TYPE_*

#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/object.hpp>         // for Object
#include <cloe/utility/geometry.hpp>         // for quaternion_from_rpy

#include "vtd_logger.hpp"  // for lane_logger

namespace vtd {

/**
 * Converts a RDB_COORD_t (x, y, z) into Eigen::Vector3d.
 *
 * \param vtd_coord to be converted
 */
Eigen::Vector3d rdb_coord_xzy_to_vector3d(RDB_COORD_t vtd_coord) {
  return Eigen::Vector3d(vtd_coord.x, vtd_coord.y, vtd_coord.z);
}

/**
 * Converts a RDB_COORD_t (r, p, h) into Eigen::Vector3d.
 *
 * \param vtd_coord to be converted
 */
Eigen::Vector3d rdb_coord_rph_to_vector3d(RDB_COORD_t vtd_coord) {
  return Eigen::Vector3d(vtd_coord.r, vtd_coord.p, vtd_coord.h);
}

/**
 * Map to convert from VTD to Cloe object classification.
 */
const std::map<int, cloe::Object::Class> vtd_object_class_map = {
    {RDB_OBJECT_TYPE_NONE, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_PLAYER_NONE, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_PLAYER_CAR, cloe::Object::Class::Car},
    {RDB_OBJECT_TYPE_PLAYER_TRUCK, cloe::Object::Class::Truck},
    {RDB_OBJECT_TYPE_PLAYER_VAN, cloe::Object::Class::Truck},
    {RDB_OBJECT_TYPE_PLAYER_BIKE, cloe::Object::Class::Bike},
    {RDB_OBJECT_TYPE_PLAYER_PEDESTRIAN, cloe::Object::Class::Pedestrian},
    {RDB_OBJECT_TYPE_PLAYER_PED_GROUP, cloe::Object::Class::Pedestrian},
    {RDB_OBJECT_TYPE_POLE, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_TREE, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_BARRIER, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_OPT1, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_OPT2, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_OPT3, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_PLAYER_MOTORBIKE, cloe::Object::Class::Motorbike},
    {RDB_OBJECT_TYPE_PLAYER_BUS, cloe::Object::Class::Truck},
    {RDB_OBJECT_TYPE_STREET_LAMP, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_TRAFFIC_SIGN, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_HEADLIGHT, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_PLAYER_TRAILER, cloe::Object::Class::Trailer},
    {RDB_OBJECT_TYPE_BUILDING, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_PARKING_SPACE, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_ROAD_WORKS, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_ROAD_MISC, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_TUNNEL, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_LEGACY, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_VEGETATION, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_MISC_MOTORWAY, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_MISC_TOWN, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_PATCH, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_OTHER, cloe::Object::Class::Unknown},
    {RDB_OBJECT_PLAYER_SEMI_TRAILER, cloe::Object::Class::Trailer},
    {RDB_OBJECT_PLAYER_RAILCAR, cloe::Object::Class::Unknown},
    {RDB_OBJECT_PLAYER_RAILCAR_SEMI_HEAD, cloe::Object::Class::Unknown},
    {RDB_OBJECT_PLAYER_RAILCAR_SEMI_BACK, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_VEH_LIGHT_FRONT_LEFT, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_VEH_LIGHT_FRONT_RIGHT, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_VEH_LIGHT_REAR_LEFT, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_VEH_LIGHT_REAR_RIGHT, cloe::Object::Class::Unknown},
    {RDB_OBJECT_TYPE_VEH_CABIN, cloe::Object::Class::Unknown},
};

Eigen::Isometry3d from_vtd_pose(const RDB_COORD_t& x) {
  Eigen::Quaterniond quaternion = cloe::utility::quaternion_from_rpy(x.r, x.p, x.h);
  Eigen::Vector3d translation = rdb_coord_xzy_to_vector3d(x);
  return cloe::utility::pose_from_rotation_translation(quaternion, translation);
}

void from_vtd_object_state(const RDB_OBJECT_STATE_t* rdb_os, bool ext, cloe::Object& object) {
  object.id = rdb_os->base.id;
  object.type = cloe::Object::Type::Static;
  object.classification = vtd_object_class_map.at(rdb_os->base.type);

  // convert pose
  object.pose = from_vtd_pose(rdb_os->base.pos);
  object.dimensions = Eigen::Vector3d{
      rdb_os->base.geo.dimX,
      rdb_os->base.geo.dimY,
      rdb_os->base.geo.dimZ,
  };
  object.cog_offset = Eigen::Vector3d{
      rdb_os->base.geo.offX,
      rdb_os->base.geo.offY,
      rdb_os->base.geo.offZ,
  };

  if (ext) {
    object.type = cloe::Object::Type::Dynamic;
    object.acceleration = rdb_coord_xzy_to_vector3d(rdb_os->ext.accel);
    object.velocity = rdb_coord_xzy_to_vector3d(rdb_os->ext.speed);
    object.angular_velocity = rdb_coord_rph_to_vector3d(rdb_os->ext.speed);
  }
}

/**
 * Map to convert from VTD roadmark types to Cloe lane boundary types.
 */
const std::map<int, cloe::LaneBoundary::Type> vtd_roadmark_type_map = {
    {RDB_ROADMARK_TYPE_NONE, cloe::LaneBoundary::Type::Unknown},
    {RDB_ROADMARK_TYPE_SOLID, cloe::LaneBoundary::Type::Solid},
    {RDB_ROADMARK_TYPE_BROKEN, cloe::LaneBoundary::Type::Dashed},
    {RDB_ROADMARK_TYPE_CURB, cloe::LaneBoundary::Type::Curb},
    {RDB_ROADMARK_TYPE_GRASS, cloe::LaneBoundary::Type::Grass},
    {RDB_ROADMARK_TYPE_BOTDOT, cloe::LaneBoundary::Type::Unknown},
    {RDB_ROADMARK_TYPE_OTHER, cloe::LaneBoundary::Type::Unknown},
};

/**
 * Map to convert from VTD roadmark colors to Cloe lane boundary colors.
 */
const std::map<int, cloe::LaneBoundary::Color> vtd_roadmark_color_map = {
    {RDB_ROADMARK_COLOR_NONE, cloe::LaneBoundary::Color::Unknown},
    {RDB_ROADMARK_COLOR_WHITE, cloe::LaneBoundary::Color::White},
    {RDB_ROADMARK_COLOR_RED, cloe::LaneBoundary::Color::Red},
    {RDB_ROADMARK_COLOR_YELLOW, cloe::LaneBoundary::Color::Yellow},
    {RDB_ROADMARK_COLOR_OTHER, cloe::LaneBoundary::Color::Unknown},
    {RDB_ROADMARK_COLOR_BLUE, cloe::LaneBoundary::Color::Blue},
    {RDB_ROADMARK_COLOR_GREEN, cloe::LaneBoundary::Color::Green},
};

void from_vtd_roadmark(const RDB_ROADMARK_t* rm, cloe::LaneBoundary& lb) {
  lb.id = rm->id;
  lb.prev_id = rm->prevId;
  lb.next_id = rm->nextId;
  lb.dx_start = rm->startDx;
  lb.dy_start = rm->lateralDist;
  lb.heading_start = rm->yawRel;
  lb.curv_hor_start = rm->curvHor;
  lb.curv_hor_change = rm->curvHorDot;
  lb.dx_end = rm->previewDx;
  lb.type = vtd_roadmark_type_map.at(rm->type);
  lb.color = vtd_roadmark_color_map.at(rm->color);

  const RDB_POINT_t* current_point = reinterpret_cast<const RDB_POINT_t*>(rm + 1);
  for (int i = 0; i < rm->noDataPoints; ++i, current_point++) {
    lb.points.push_back(Eigen::Vector3d(current_point->x, current_point->y, current_point->z));
  }

  // clang-format off
  lane_logger()->trace(
      "# {: 2d}  "
      "<{: 2d} | {: 2d}> "
      "[{: 3.3f}, {: 3.3f}] "
      "dy: {: 3.3f} "
      "curv: {: 1.4f} {: 1.4f} {: 1.4f}",
      lb.id,
      lb.prev_id, lb.next_id,
      lb.dx_start, lb.dx_end,
      lb.dy_start,
      lb.heading_start, lb.curv_hor_start, lb.curv_hor_change
  ); // NOLINT
  // clang-format on
}

}  // namespace vtd
