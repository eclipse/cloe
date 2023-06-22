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

#include <map>     // for map
#include <vector>  // for vector

#include <Eigen/Geometry>  // for Vector3d

#include <fable/confable.hpp> // for Confable
#include <fable/fable_fwd.hpp> // for Schema
#include <fable/enum.hpp> // for ENUM_SERIALIZATION

namespace cloe {

class LaneBoundary : public fable::Confable {
 public:
  /**
   * Type of lane boundary.
   */
  enum class Type { Unknown, Solid, Dashed, Grass, Curb };
  friend void to_json(fable::Json& j, const LaneBoundary::Type& t);
  friend void from_json(const fable::Json& j, LaneBoundary::Type& t);

  /**
   * Color of the lane boundary.
   */
  enum class Color { Unknown, White, Yellow, Red, Green, Blue };
  friend void to_json(fable::Json& j, const LaneBoundary::Color& t);
  friend void from_json(const fable::Json& j, LaneBoundary::Color& t);

 public:
  fable::Schema schema_impl() override;
  void to_json(fable::Json& j) const override;

  CONFABLE_FRIENDS(LaneBoundary)

 public:
  int id{-1};
  int prev_id{-1};
  int next_id{-1};
  double dx_start{0};
  double dy_start{0};
  double heading_start{0};
  double curv_hor_start{0};
  double curv_hor_change{0};
  double dx_end{0};
  double exist_prob{0};
  Type type{Type::Unknown};
  Color color{Color::Unknown};

  std::vector<Eigen::Vector3d> points;
};

using LaneBoundaries = std::map<int, LaneBoundary>;
void to_json(fable::Json& j, const LaneBoundaries& lbs);

// clang-format off
ENUM_SERIALIZATION(LaneBoundary::Type, ({
    {LaneBoundary::Type::Unknown, "unknown"},
    {LaneBoundary::Type::Solid, "solid"},
    {LaneBoundary::Type::Dashed, "dashed"},
    {LaneBoundary::Type::Grass, "grass"},
    {LaneBoundary::Type::Curb, "curb"},
}))

ENUM_SERIALIZATION(LaneBoundary::Color, ({
    {LaneBoundary::Color::Unknown, "unknown"},
    {LaneBoundary::Color::White, "white"},
    {LaneBoundary::Color::Yellow, "yellow"},
    {LaneBoundary::Color::Red, "red"},
    {LaneBoundary::Color::Green, "green"},
    {LaneBoundary::Color::Blue, "blue"},
}))
// clang-format on

}  // namespace cloe
