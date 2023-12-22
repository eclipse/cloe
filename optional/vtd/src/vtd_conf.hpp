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
 * \file conf.hpp
 */

#pragma once

#include <chrono>  // for duration<>
#include <map>     // for map<>
#include <memory>  // for shared_ptr<>
#include <string>  // for string

#include <cloe/core.hpp>                            // for Conf, Schema
#include <cloe/utility/tcp_transceiver_config.hpp>  // for TcpTransceiverConfiguration, ...
#include <osi/utility/osi_omni_sensor.hpp>          // for SensorMockLevel

// Connection / Initialization
#define VTD_DEFAULT_SCP_PORT 48179
#define VTD_PARAMSERVER_PORT 54345
#define VTD_INIT_SYNC_SLEEP_MS 3000
#define VTD_INIT_WAIT_SLEEP_MS 200

namespace vtd {

/**
 * The LabelConfiguration class lets you configure how we let VTD label
 * vehicles.
 */
enum class LabelConfiguration { Off, Text, Human, Symbol, Unicode };

// clang-format off
ENUM_SERIALIZATION(LabelConfiguration, ({
  {LabelConfiguration::Off, "off"},
  {LabelConfiguration::Text, "text"},
  {LabelConfiguration::Human, "human"},
  {LabelConfiguration::Symbol, "symbol"},
  {LabelConfiguration::Unicode, "unicode"},
}))
// clang-format on

/**
 * The ProtocolConfiguration class lets you configure how we receive
 * sensor data.
 */
enum class ProtocolConfiguration { Rdb, Osi };

// clang-format off
ENUM_SERIALIZATION(ProtocolConfiguration, ({
  {ProtocolConfiguration::Rdb, "rdb"},
  {ProtocolConfiguration::Osi, "osi"},
}))
// clang-format on

/**
 * Definition of a single vehicle's VTD sensor.
 *
 * In future we may support different formats to specify the sensor properties
 * in order to perform simple things in a simple way. Currently the only way is
 * to define it in terms of the VTD module manager XML configuration as
 * described in the following section.
 */
struct VtdSensorConfig : public cloe::Confable {
  /**
   * VTD module manager XML configuration for a single VTD sensor.
   *
   * The XML can be used to configure mounting position and orientation, the
   * sensor's frustum, and filters defining which types of objects are
   * perceived.
   *
   * The following placeholders are interpolated with runtime data:
   *
   * - [[ sensor_id ]] Sensor id to create a unique sensor name in <Sensor>
   * - [[ sensor_name ]] Sensor name to create a speaking sensor name in <Sensor>
   * - [[ sensor_port ]] TCP port for the sensor's RDB channel in <Port>
   * - [[ player_id ]] Player id for <Player>
   */
  std::string xml = "";

  ProtocolConfiguration protocol = ProtocolConfiguration::Rdb;

  /**
   * Overwrite data by ground truth.
   * Currently supported for OSI protocol only.
   */
  std::shared_ptr<osii::SensorMockConf> sensor_mock_conf = std::make_shared<osii::SensorMockConf>();

 public:
  CONFABLE_SCHEMA(VtdSensorConfig) {
    // clang-format off
    return cloe::Schema{
        {"xml",         cloe::Schema(&xml, "VTD module manager sensor configuration")},
        {"protocol",    cloe::Schema(&protocol, "VTD module manager sensor connection protocol ( rdb | osi )")},
        {"mock_level",  cloe::Schema(sensor_mock_conf.get(), "Sensor data mock level")},
    };
    // clang-format on
  }
};

/**
 * Sensor and component type definition.
 * Defines the exact sensor and selects a component type to instantiate.
 */
struct VtdComponentConfig : public cloe::Confable {
  /**
   * Sensor name
   *
   * Refers to the sensorname as defined in sensors section. This defines the
   * VTD sensor to take a particular type of data (i.e. this component) from.
   * The VTD sensor names are defined by the keys in the sensors configuration.
   */
  std::string from;

  /**
   * Component type
   *
   * One of
   * - lane_sensor
   * - object_sensor
   */
  std::string type;

  /**
   * Override an existing sensor with that name.
   *
   * If a component with that name already exists this can be set to true to
   * override the existing component.
   *
   * Defaults to false.
   */
  bool override = false;

 public:
  CONFABLE_SCHEMA(VtdComponentConfig) {
    return cloe::Schema{
        {"from", cloe::Schema(&from, "VTD sensor to retrieve the component data from")},
        {"type", cloe::Schema(&type, "Component type to register")},
        {"override", cloe::Schema(&override, "Override an existing component with the same name")},
    };
  };
};

/**
 * VtdVehicleConfig contains the vtd specific vehicle configuration.
 *
 * That is, sensor definitions and a mapping to cloe components.
 */
struct VtdVehicleConfig : public cloe::Confable {
  /**
   * Sensor definitions
   */
  std::map<std::string, VtdSensorConfig> sensors;

  /**
   * Component name -> Sensor + component type mappings
   */
  std::map<std::string, VtdComponentConfig> components;

 public:
  CONFABLE_SCHEMA(VtdVehicleConfig) {
    return cloe::Schema{
        {"sensors", cloe::Schema(&sensors, "sensor definitions")},
        {"components", cloe::Schema(&components, "component definitions")},
    };
  }
};

/**
 * The VtdConfiguration class contains all configuration values for VTD.
 * It can be merged from an input JSON object, as well as serialized to a JSON object.
 */
struct VtdConfiguration : public cloe::Confable {
  /**
   * Connection parameters for the SCP connection, including host and port.
   */
  cloe::utility::TcpTransceiverFullConfiguration paramserver{"localhost", VTD_PARAMSERVER_PORT};

  /**
   * Connection parameters for the SCP connection, including host and port.
   */
  cloe::utility::TcpTransceiverFullConfiguration connection{"localhost", VTD_DEFAULT_SCP_PORT};

  /**
   * Connection parameters for the Task Control client connection.
   */
  cloe::utility::TcpTransceiverConfiguration task_control_params{60,
                                                                 std::chrono::duration<float>{0.5}};

  /**
   * Connection parameters for the RDB sensor connections.
   */
  cloe::utility::TcpTransceiverConfiguration rdb_params{60, std::chrono::duration<float>{0.5}};

  /**
   * Attempt to recover configuration failure this many times.
   *
   * This commonly occurs with VTD; in certain circumstances up to 50% of
   * configurations fail with a broken TCP pipe. By retrying N times, we
   * increase the chances of success to 1 - 1/2^N.
   */
  uint16_t configuration_retry_attempts{10};

  /**
   * TCP port for the first sensor RDB connection.
   *
   * Because these parameters are used not only for creating multiple vehicles,
   * but also for each sensor in a vehicle, the port is only used for
   * the very first sensor configured across all vehicles, and is subsequently
   * incremented for each opened RDB sensor connection.
   */
  uint16_t sensor_initial_port{48196};

  /**
   * Vehicle parameters such as sensor definitions and component mappings
   */
  std::map<std::string, VtdVehicleConfig> vehicles;

  std::string setup = "Cloe.Default";

  std::string scenario = "";
  std::string project = "";

  /**
   * Whether to use the VTD image generator.
   *
   * This is also derived to be false when the setup is "Cloe.noGUInoIG" or
   * "Cloe.noIG".
   */
  bool image_generator = true;

  /**
   * Whether to use the third person for the camera.
   * Currently, this is a prerequisite for setting the camera at all.
   */
  bool camera_third_person = true;

  /**
   * What player to focus the camera on, by name.
   * If this is empty or not found, a random player is taken.
   */
  std::string camera_focus_on = "";

  /**
   * Whether to label the ego vehicle controller state.
   */
  LabelConfiguration label_vehicle = LabelConfiguration::Text;

  /**
   * Record VTD dat file.
   */
  std::string dat_file = "";

  /**
   * A set of predefined SCP actions to be used by the action trigger.
   */
  std::map<std::string, std::string> scp_actions{};

 public:
  CONFABLE_SCHEMA(VtdConfiguration) {
    // clang-format off
    using namespace cloe;  // NOLINT(build/namespaces)
    return Schema{
        {"connection",          Schema(&connection, "scp connection parameters")},
        {"paramserver",         Schema(&paramserver, "parameter sever connection parameters")},
        {"task_control_params", Schema(&task_control_params, "task control connection parameters")},
        {"rdb_params",          Schema(&rdb_params, "rdb connection parameters")},
        {"sensor_initial_port", Schema(&sensor_initial_port, "initial port for sensor communication")},
        {"vehicles",            Schema(&vehicles, "vehicle configuration like sensors and component mapping")},
        {"configuration_retry_attempts", Schema(&configuration_retry_attempts, "attempts to retry connection on broken pipe")},
        {"setup",               Schema(&setup, "indicate which setup you are using")},
        {"image_generator",     Schema(&image_generator, "switch whether VTD should use image generator")},
        {"scenario",            Schema(&scenario, "VTD scenario to use (project must already be loaded)")},
        {"project",             Schema(&project, "indicate which project to find the scenario in (informative)")},
        {"label_vehicle",       Schema(&label_vehicle, "how to label vehicle modes in VTD [off,text,human,symbol,unicode]")},
        {"dat_file",            Schema(&dat_file, "filepath to write VTD data output to")},
        {"scp_actions",         Schema(&scp_actions, "predefined SCP actions for use by action trigger")},
        {"camera",              Schema({
                                      {"third_person",Schema(&camera_third_person, "whether to use third person camera")},
                                      {"focus_on",    Schema(&camera_focus_on, "player to focus on")},
                                })},
    };
    // clang-format on
  }
};

}  // namespace vtd
