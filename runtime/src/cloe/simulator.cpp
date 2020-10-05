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
 * \file cloe/simulator.cpp
 * \see  cloe/simulator.hpp
 */

#include <cloe/simulator.hpp>
#include <cloe/vehicle.hpp>

namespace cloe {

void to_json(Json& j, const Simulator& b) {
  Json vehicles = Json::array();
  for (size_t i = 0; i < b.num_vehicles(); i++) {
    auto v = b.get_vehicle(i);
    if (v != nullptr) {
      vehicles.push_back(*v);
    }
  }

  j = Json{
      {"is_connected", b.is_connected()},
      {"is_operational", b.is_operational()},
      {"num_vehicles", b.num_vehicles()},
      {"vehicles", vehicles},
  };
}

}  // namespace cloe
