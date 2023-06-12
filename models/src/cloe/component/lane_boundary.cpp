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

#include <fable/utility/eigen.hpp>  // for to_json

#include <fable/json.hpp> // for Json
#include <fable/schema.hpp> // for Schema, Struct

namespace cloe {

void LaneBoundary::to_json(fable::Json& j) const {
  j = fable::Json{
      {"id", id},
      {"prev_id", prev_id},
      {"next_id", next_id},
      {"dx_start", dx_start},
      {"dy_start", dy_start},
      {"heading_start", heading_start},
      {"curv_hor_start", curv_hor_start},
      {"curv_hor_change", curv_hor_change},
      {"dx_end", dx_end},
      {"exist_prob", exist_prob},
      {"type", type},
      {"color", color},
      {"points", points},
  };
}

fable::Schema LaneBoundary::schema_impl() {
  using namespace fable;
  return schema::Struct{
      // clang-format off
      {"id", make_schema(&id, "unique identifier in scene graph")},
      {"prev_id", make_schema(&prev_id, "previous identifier")},
      {"next_id", make_schema(&next_id, "next identifier")},
      {"dx_start", make_schema(&dx_start, "start of lane boundary in ego x-direction [m]")},
      {"dy_start", make_schema(&dy_start, "lateral distance to ego vehicle reference point [m]")},
      {"heading_start", make_schema(&heading_start, "yaw angle relative to ego x-direction [rad]")},
      {"curv_hor_start", make_schema(&curv_hor_start, "horizontal curvature at lane boundary start [1/m]")},
      {"curv_hor_change", make_schema(&curv_hor_change, "change of horizontal curvature at lane boundary start [1/m^2]")},
      {"dx_end", make_schema(&dx_end, "end of lane boundary in ego x-direction [m]")},
      {"exist_prob", make_schema(&exist_prob, "existence probability")},
      {"type", make_schema(&type, "lane boundary type")},
      {"color", make_schema(&color, "lane boundary color")},
      // clang-format on
  }
      .require_all();
}

void to_json(fable::Json& j, const LaneBoundaries& lbs) {
  for (auto lb_pair : lbs) {
    j[std::to_string(lb_pair.first)] = lb_pair.second;
  }
}

}  // namespace cloe
