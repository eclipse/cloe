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
 * \file cloe/component/lane_boundary.hpp
 * \see  cloe/component/lane_boundary.cpp
 */

#pragma once
#ifndef CLOE_COMPONENT_LANE_BOUNDARY_HPP_
#define CLOE_COMPONENT_LANE_BOUNDARY_HPP_

#include <map>     // for map
#include <vector>  // for vector

#include <Eigen/Geometry>  // for Vector3d

#include <cloe/core.hpp>  // for Confable, Json, Schema

namespace cloe {

class LaneBoundary : public Confable {
 public:
  /**
   * Type of lane boundary.
   */
  enum class Type { Unknown, Solid, Dashed, Grass, Curb };
  friend void to_json(Json& j, const LaneBoundary::Type& t);
  friend void from_json(const Json& j, LaneBoundary::Type& t);

  /**
   * Color of the lane boundary.
   */
  enum class Color { Unknown, White, Yellow, Red, Green, Blue };
  friend void to_json(Json& j, const LaneBoundary::Color& t);
  friend void from_json(const Json& j, LaneBoundary::Color& t);

 public:
  Schema schema_impl() override;
  void to_json(Json& j) const override;

  CONFABLE_FRIENDS(LaneBoundary)

 public:
  int id;
  int prev_id;
  int next_id;
  double dx_start;
  double dy_start;
  double heading_start;
  double curv_hor_start;
  double curv_hor_change;
  double dx_end;
  Type type;
  Color color;

  std::vector<Eigen::Vector3d> points;
};

using LaneBoundaries = std::map<int, LaneBoundary>;
void to_json(Json& j, const LaneBoundaries& lbs);

}  // namespace cloe

#endif  // CLOE_COMPONENT_LANE_BOUNDARY_HPP_
