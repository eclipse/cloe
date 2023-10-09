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
 * \file cloe/component/object.hpp
 */

#pragma once

#include <memory>  // for shared_ptr<>
#include <vector>  // for vector<>

#include <sol/sol.hpp>  // for Lua related aspects

#include <Eigen/Geometry>           // for Isometry3d, Vector3d
#include <fable/enum.hpp>           // for ENUM_SERIALIZATION
#include <fable/json.hpp>           // for Json
#include <fable/utility/eigen.hpp>  // for to_json

namespace cloe {

/**
 * Object represents an object in 3D space.
 *
 * An object is a 3D bounding box with velocity, angular velocity, position,
 * orientation, and acceleration.
 *
 * Note: It is a POD and therefore nothing should inherit from it.
 *
 * Approximate size of Object is: 168 bytes
 *
 * TODO(ben): In one documentation from Eigen that I read, Eigen objects should not be copied, but
 * passed by reference. If there is substance to this claim, then that would mean that an Object
 * should also only be created on the heap. I have a hard time believing this, but maybe it's worth
 * investigating.
 */
struct Object {
  enum class Type { Unknown, Static, Dynamic };

  enum class Class { Unknown, Pedestrian, Bike, Motorbike, Car, Truck, Trailer };

  /// ID of object, should be unique.
  int id{-1};

  /// Object existence probability.
  double exist_prob{1.0};

  /// Type of object.
  Type type{Type::Unknown};

  /// Classification of object.
  Class classification{Class::Unknown};

  /// Pose in [m] and [rad].
  Eigen::Isometry3d pose{Eigen::Isometry3d()};

  /// Dimensions in [m].
  Eigen::Vector3d dimensions{Eigen::Vector3d::Zero()};

  /// Center of geometry offset in [m].
  Eigen::Vector3d cog_offset{Eigen::Vector3d::Zero()};

  /// Absolute velocity in [m/s].
  Eigen::Vector3d velocity{Eigen::Vector3d::Zero()};

  /// Absolute acceleration in [m/(s*s)].
  Eigen::Vector3d acceleration{Eigen::Vector3d::Zero()};

  /// Angular velocity in [rad/s].
  Eigen::Vector3d angular_velocity{Eigen::Vector3d::Zero()};

  Object() = default;

  friend void to_json(fable::Json& j, const Object& o) {
    j = fable::Json{
        {"id", o.id},
        {"exist_prob", o.exist_prob},
        {"type", o.type},
        {"class", o.classification},
        {"pose", o.pose},
        {"dimensions", o.dimensions},
        {"cog_offset", o.cog_offset},
        {"velocity", o.velocity},
        {"velocity_norm", static_cast<double>(o.velocity.norm())},
        {"acceleration", o.acceleration},
        {"angular_velocity", o.angular_velocity},
    };
  }
  friend void to_lua(sol::state_view view, Object* /* value */) {
    sol::usertype<Object> usertype_table = view.new_usertype<Object>("Object");
    usertype_table["id"] = &Object::id;
    usertype_table["exist_prob"] = &Object::exist_prob;
    usertype_table["type"] = &Object::type;
    usertype_table["classification"] = &Object::classification;
    usertype_table["pose"] = &Object::pose;
    usertype_table["dimensions"] = &Object::dimensions;
    usertype_table["cog_offset"] = &Object::cog_offset;
    usertype_table["velocity"] = &Object::velocity;
    usertype_table["acceleration"] = &Object::acceleration;
    usertype_table["angular_velocity"] = &Object::angular_velocity;
    usertype_table["ego_position"] = +[](const Object &self, const Eigen::Isometry3d &sensorMountPose) {
        Eigen::Vector3d pos = sensorMountPose * self.pose * self.cog_offset;
        return pos;
    };
  }
};

/**
 * Objects is an alias for a vector of objects.
 *
 * This is defined as many components in the simulation take or return
 * a collection of Objects.
 */
using Objects = std::vector<std::shared_ptr<Object>>;

inline void to_json(fable::Json& j, const Objects& os) {
  j = fable::Json::array();
  for (const auto& o : os) {
    assert(o != nullptr);
    j.push_back(*o);
  }
}

// clang-format off
ENUM_SERIALIZATION(Object::Type, ({
      {Object::Type::Static, "static"},
      {Object::Type::Dynamic, "dynamic"},
      {Object::Type::Unknown, "unknown"},
}))

ENUM_SERIALIZATION(Object::Class, ({
      {Object::Class::Unknown, "unknown"},
      {Object::Class::Pedestrian, "pedestrian"},
      {Object::Class::Bike, "bicycle"},
      {Object::Class::Motorbike, "motorcycle"},
      {Object::Class::Car, "car"},
      {Object::Class::Truck, "truck"},
      {Object::Class::Trailer, "trailer"},
}))
// clang-format on

}  // namespace cloe
