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
 * \file cloe/component/utility/ego_sensor_canon.cpp
 * \see  cloe/component/utility/ego_sensor_canon.hpp
 */

#include <cloe/component/utility/ego_sensor_canon.hpp>

#include <limits>  // for numeric_limits<>
#include <memory>  // for shared_ptr<>

namespace cloe {
namespace utility {

std::shared_ptr<Object> closest_forward(const Objects objects) {
  double current_dist = std::numeric_limits<double>::max();
  std::shared_ptr<Object> current_obj;
  for (const auto& o : objects) {
    if (is_same_lane(*o) && is_object_fore(*o)) {
      auto d = distance_forward(*o);
      if (d < current_dist) {
        current_dist = d;
        current_obj = o;
      }
    }
  }
  return current_obj;
}

}  // namespace utility
}  // namespace cloe
