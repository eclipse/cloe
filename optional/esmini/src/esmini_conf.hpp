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
 * \file esmini_conf.hpp
 */

#pragma once

#include <chrono>  // for duration<>
#include <map>     // for map<>
#include <memory>  // for shared_ptr<>
#include <string>  // for string

#include <cloe/core.hpp>  // for Conf, Schema

namespace esmini {

/**
 * ESMiniVehicleConfig contains the esmini specific vehicle configuration.
 *
 * That is, sensor definitions and a mapping to cloe components.
 */
struct ESMiniVehicleConfig : public cloe::Confable {
  /// Externally controlled esmini vehicle.
  bool is_closed_loop{true};

  /// Only keep ground truth data within given distance.
  double filter_dist{100.0};

 public:
  CONFABLE_SCHEMA(ESMiniVehicleConfig) {
    // clang-format off
    return cloe::Schema{
        {"closed_loop", cloe::Schema(&is_closed_loop, "control the esmini vehicle")},
        {"filter_distance", cloe::Schema(&filter_dist, "filter distance for ground truth data")},
    };
    // clang-format on
  }
};

/**
 * The ESMiniConfiguration class contains all configuration values for ESMini.
 * It can be merged from an input JSON object, as well as serialized to a JSON object.
 */
struct ESMiniConfiguration : public cloe::Confable {
  std::string scenario = "";

  bool is_headless{true};

  bool write_images{false};

  /**
   * Vehicle parameters such as sensor definitions and component mappings.
   */
  std::map<std::string, ESMiniVehicleConfig> vehicles;

  CONFABLE_SCHEMA(ESMiniConfiguration) {
    // clang-format off
    return cloe::Schema{
        {"headless", cloe::Schema(&is_headless, "run esmini without viewer")},
        {"write_images", cloe::Schema(&write_images, "save an image for each step")},
        {"scenario", cloe::Schema(&scenario, "absolute path to open scenario file")},
        {"vehicles", cloe::Schema(&vehicles, "vehicle configuration like sensors and component mapping")},
    };
    // clang-format on
  }
};

}  // namespace esmini
