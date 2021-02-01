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
 * \file scp_messages.hpp
 * \see  scp_messages.cpp
 *
 * This header file contains all important definitions for SCP communication
 * messages with VTD.
 */

#pragma once

#include <string>  // for string

#include <boost/filesystem/path.hpp>  // for path

#include "scp_transceiver.hpp"  // for ScpMessage

namespace vtd {
namespace scp {

extern const char* Start;
extern const char* Stop;
extern const char* Pause;
extern const char* Restart;
extern const char* Apply;
extern const char* Config;
extern const char* QueryInit;
extern const char* AckInit;
extern const char* InitOperation;

struct ParamServerConfig : public ScpMessage {
  std::string sync_source = "RDB";
  bool no_image_generator = false;
  std::string to_scp() const override;
};

struct ScenarioConfig : public ScpMessage {
  std::string filename;
  std::string to_scp() const override;
};

struct CameraPosition : public ScpMessage {
  std::string tethered_to_player;
  std::string look_to_player;

  std::string to_scp() const override;
};

struct SensorConfiguration : public ScpMessage {
  int port;
  int player_id;
  int sensor_id;
  std::string to_scp() const override;
};

struct DynamicsPluginConfig : public ScpMessage {
  std::string name;
  std::string to_scp() const override;
};

struct LabelVehicle : public ScpMessage {
  std::string tethered_to_player;
  std::string text;
  std::string color = "0xFF0000";

  /**
   * The height above the vehicle at which the label is anchored.
   * A height <= 1.0 likely results in the label not being shown.
   */
  double dz_m = 2.0;

  std::string to_scp() const override;
};

struct RecordDat : public ScpMessage {
  boost::filesystem::path datfile_path;
  std::string to_scp() const override;
};

struct QueryScenario : public ScpMessage {
  std::string scenario;
  std::string to_scp() const override;
};

}  // namespace scp
}  // namespace vtd
