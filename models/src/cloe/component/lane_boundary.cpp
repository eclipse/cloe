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
 * \file cloe/component/lane_boundary.cpp
 * \see  cloe/component/lane_boundary.hpp
 */

#include <cloe/component/lane_boundary.hpp>

#include <map>          // for map<>
#include <stdexcept>    // for runtime_error
#include <string>       // for string
#include <type_traits>  // for underlying_type<>

#include <fable/json/with_eigen.hpp>  // for to_json

#include <cloe/core.hpp>  // for Json, Schema

namespace cloe {

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

void LaneBoundary::to_json(Json& j) const {
  j = Json{
      {"id", id},
      {"prev_id", prev_id},
      {"next_id", next_id},
      {"dx_start", dx_start},
      {"dy_start", dy_start},
      {"heading_start", heading_start},
      {"curv_hor_start", curv_hor_start},
      {"curv_hor_change", curv_hor_change},
      {"dx_end", dx_end},
      {"type", type},
      {"color", color},
      {"points", points},
  };
}

Schema LaneBoundary::schema_impl() {
  return schema::Struct{
      {"id", Schema(&id, "unique identifier in scene graph")},
      {"prev_id", Schema(&prev_id, "previous identifier")},
      {"next_id", Schema(&next_id, "next identifier")},
      {"dx_start", Schema(&dx_start, "")},
      {"dy_start", Schema(&dy_start, "")},
      {"heading_start", Schema(&heading_start, "")},
      {"curv_hor_start", Schema(&curv_hor_start, "")},
      {"curv_hor_change", Schema(&curv_hor_change, "")},
      {"dx_end", Schema(&dx_end, "")},
      {"type", Schema(&type, "lane boundary type")},
      {"color", Schema(&color, "lane boundary color")},
  }
      .require_all();
}

void to_json(Json& j, const LaneBoundaries& lbs) {
  for (auto lb_pair : lbs) {
    j[std::to_string(lb_pair.first)] = lb_pair.second;
  }
}

}  // namespace cloe
