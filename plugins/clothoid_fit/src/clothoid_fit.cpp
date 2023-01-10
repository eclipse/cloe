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
 * \file clothoid_fit.cpp
 * \see  clothoid_fit.hpp
 *
 */

#include "clothoid_fit.hpp"

#include <cloe/component.hpp>              // for Component, Json
#include <cloe/component/lane_sensor.hpp>  // for LaneBoundarySensor
#include <cloe/plugin.hpp>                 // for EXPORT_CLOE_PLUGIN

namespace cloe {

namespace component {

DEFINE_COMPONENT_FACTORY(ClothoidFitFactory, ClothoidFitConf, "clothoid_fit",
                         "fit clothoids to polylines")

DEFINE_COMPONENT_FACTORY_MAKE(ClothoidFitFactory, LaneBoundaryClothoidFit, LaneBoundarySensor)

}  // namespace component
}  // namespace cloe

// Register factory as plugin entrypoint
EXPORT_CLOE_PLUGIN(cloe::component::ClothoidFitFactory)
