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
 */
/**
 * \file sumo.hpp
 */

#pragma once
#include <string>              // for string
#include <cloe/core.hpp>       // for Conf, Schema
#include <cloe/simulator.hpp>  // for DEFINE_SIMULATOR_FACTORY

namespace cloe {
namespace sumo {

/**
 * The SumoConfiguration struct contains all configuration values for
 * Sumo.
 *
 * It can be merged from an input JSON object, as well as serialized to a JSON
 * object.
 */
struct SumoConfiguration : public Confable {
  /**
   * The endpoint where the Sumo simulator binding is listening for
   * connections from the Sumo simulation.
   *
   * Note that this cannot be the default Sumo executable, but the one with
   * CloeVehicleCtrl module built-in and each TestRun's Vehicle must be
   * configured to use that the CloeVehicleCtrl module.
   */
  std::string ip_addr;
  int port;
  std::string input_cfg_file;
  std::vector<std::string> egos;

 public:
  CONFABLE_SCHEMA(SumoConfiguration) {
    return Schema{{"ip_addr", Schema(&ip_addr, "IP Address for connecting to Sumo")},
                  {"port", Schema(&port, "Port On Which Sumo is Running")},
                  {"input_cfg_file", Schema(&input_cfg_file, "Sumo Configuration file")},
                  {"egos", Schema(&egos, "Ego Vehicle")}};
  }
};

}  // namespace sumo
}  // namespace cloe
