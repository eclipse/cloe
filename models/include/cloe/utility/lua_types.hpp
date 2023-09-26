/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file cloe/component/ego_sensor.hpp
 */

#pragma once

#include <cloe/data_broker.hpp>

#include <Eigen/Dense>

#include <cloe/component/wheel.hpp>

namespace cloe {
namespace utility {

extern void register_lua_types(cloe::DataBroker& db);

extern void register_gaspedal_sensor(DataBroker& db, const std::string& vehicle,
                                     std::function<const double&()> gaspedal_getter);
extern void register_wheel_sensor(DataBroker& db,
                                  const std::string& vehicle,
                                  const std::string& wheel_name,
                                  std::function<const ::cloe::Wheel&()>
                                      wheel_getter);

}  // namespace utility
}  // namespace cloe
