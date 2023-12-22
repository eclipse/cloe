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
 * \file vtd_vehicle.hpp
 */

#pragma once

#include <algorithm>  // for transform, min
#include <map>        // for map<>
#include <memory>     // for unique_ptr<>, shared_ptr<>
#include <string>     // for string, to_string
#include <thread>     // for this_thread

#include <cloe/component/object_sensor_functional.hpp>  // for ObjectSensorFilter
#include <cloe/core.hpp>                                // for Duration
#include <cloe/models.hpp>                              // for CloeComponent
#include <cloe/sync.hpp>                                // for Sync
#include <cloe/utility/inja.hpp>                        // for inja
#include <cloe/utility/tcp_transceiver_config.hpp>      // for TcpTransceiverConfiguration
#include <cloe/vehicle.hpp>                             // for Vehicle

#include <osi/utility/osi_transceiver_tcp.hpp>  // for OsiTransceiverTcpFactory
#include "actuator_component.hpp"               // for VtdLatLongActuator
#include "omni_sensor_component.hpp"            // for VtdOmniSensor
#include "osi_sensor_component.hpp"             // for VtdOsiSensor
#include "rdb_transceiver_tcp.hpp"              // for RdbTransceiverTcp, RdbTransceiverTcpFactory
#include "scp_messages.hpp"                     // for scp::{LabelVehicle, SensorConfiguration}
#include "scp_transceiver.hpp"                  // for ScpTransceiver
#include "task_control.hpp"                     // for TaskControl
#include "vtd_conf.hpp"                         // for VtdVehicleConfig
#include "vtd_logger.hpp"                       // for sensors_logger
#include "vtd_sensor_components.hpp"            // VtdEgoSensor, VtdWorldSensor, ...
#include "vtd_sensor_data.hpp"                  // for VtdSensorData

namespace vtd {

/**
 * The VtdVehicle class contains all information pertaining to a vehicle
 * represented in VTD.
 */
class VtdVehicle : public cloe::Vehicle {
 public:
  virtual ~VtdVehicle() = default;
  VtdVehicle() = delete;

  /**
   * Construct a new VtdVehicle.
   *
   * \param id A globally unique identifier number for this vehicle.
   *
   * \param name The VTD scenario name of this vehicle.
   *        This can then be retrieved with the `vtd_name()` method. This is
   *        different from what is returned by `name()`, because the VTD name of
   *        a vehicle does not need to conform by the identifier requirements
   *        that apply to `name()`.
   *
   * \param rdb_client RDB client unique to this vehicle.
   *        VTD creates an RDB communication channel for each vehicle in the
   *        scenario that we receive sensor data for. This channel is unique
   *        to the vehicle.
   *
   * \param task_control Task Control client shared with all vehicles.
   *        In order to transmit actuator data, we need to use the groundtruth
   *        communication channel that VTD provides. This is also the channel
   *        where the global groundtruth data is transmitted. We don't
   *        currently use this for retrieving ground truth data however, but
   *        may in the future.
   */
  VtdVehicle(uint64_t id, const std::string& name, std::unique_ptr<RdbTransceiverTcp>&& rdb_client,
             std::shared_ptr<TaskControl> task_control)
      : Vehicle(id, fmt::format("vtd_vehicle_{}", id))
      , vtd_name_(name)
      , id_(id)
      , task_control_(task_control) {
    // clang-format off
    sensor_port_ = rdb_client->tcp_port();
    vehicle_label_.tethered_to_player = name;
    vehicle_label_.text = "!";
    auto sensor = std::make_shared<VtdOmniSensor>(std::move(rdb_client), id);
    sensors_[DEFAULT_SENSOR_NAME] = sensor;
    sensor->set_name(name + "_omni_sensor");
    auto actuator = std::make_shared<VtdLatLongActuator>(task_control, id);
    ego_control_ = actuator;
    // Add ego sensor
    this->new_component(new VtdEgoSensor{id, sensor, task_control},
                        cloe::CloeComponent::GROUNDTRUTH_EGO_SENSOR,
                        cloe::CloeComponent::DEFAULT_EGO_SENSOR);

    // Add object sensor
    this->new_component(new VtdWorldSensor{sensor},
                        cloe::CloeComponent::DEFAULT_WORLD_SENSOR);

    // TODO(ben): Currently, we only send dynamic objects. This is to
    // compensate for the controllers that are currently using stuff.
    this->emplace_component(
        std::make_shared<cloe::ObjectSensorFilter>(
          this->get<cloe::ObjectSensor>(cloe::CloeComponent::DEFAULT_WORLD_SENSOR),
          [](const cloe::Object& obj) { return obj.type == cloe::Object::Type::Dynamic; }),
        cloe::CloeComponent::DEFAULT_WORLD_SENSOR);

    // Add groundtruth world sensor and emplace it by a static object only version
    this->new_component(new VtdWorldSensor{task_control},
                        cloe::CloeComponent::GROUNDTRUTH_WORLD_SENSOR);

    this->emplace_component(
        std::make_shared<cloe::ObjectSensorFilter>(
          this->get<cloe::ObjectSensor>(cloe::CloeComponent::GROUNDTRUTH_WORLD_SENSOR),
          [](const cloe::Object& obj) { return obj.type == cloe::Object::Type::Dynamic; }),
        cloe::CloeComponent::GROUNDTRUTH_WORLD_SENSOR);

    // Add lane-boundary sensor
    this->new_component(new VtdLaneBoundarySensor{sensor},
                        cloe::CloeComponent::GROUNDTRUTH_LANE_SENSOR,
                        cloe::CloeComponent::DEFAULT_LANE_SENSOR);

    // Add actuator
    this->add_component(actuator,
                        cloe::CloeComponent::GROUNDTRUTH_LATLONG_ACTUATOR,
                        cloe::CloeComponent::DEFAULT_LATLONG_ACTUATOR);
    // clang-format on
  }

  /**
   * Return the name VTD associates with this vehicle.
   */
  const std::string& vtd_name() const { return vtd_name_; }

  /**
   * Do everything that a vehicle needs before a step is triggered.
   *
   * This currently does everything for the actuation.
   */
  void vtd_step_vehicle_control(const cloe::Sync& sync, ScpTransceiver& tx,
                                LabelConfiguration lbl) {
    // Send actuations
    this->ego_control_->step_begin(sync);
    if (lbl != LabelConfiguration::Off) {
      this->update_label(tx, lbl);
    }
    this->ego_control_->step_end(sync);
  }

  /**
   * Do everything that a vehicle needs after a step is triggered.
   */
  cloe::Duration vtd_step_sensors(const cloe::Sync& s) {
    cloe::Duration d = s.time();
    for (auto kv : sensors_) {
      auto sensor = kv.second;
      sensor->step(s);
      d = std::min(sensor->time(), d);
    }
    return this->sensors_[DEFAULT_SENSOR_NAME]->time();
  }

  /**
   * If state is different from previous, then we update the LabelVehicle and
   * send the info.
   *
   * This has to been done after all actuations have been made but before
   * the cache is cleared for the next cycle.
   */
  void update_label(ScpTransceiver& tx, LabelConfiguration lbl) {
    if (ego_control_->update_vehicle_label()) {
      auto level = ego_control_->get_actuation_level();
      switch (lbl) {
        case LabelConfiguration::Text:
          vehicle_label_.text = level.to_loud_cstr();
          break;
        case LabelConfiguration::Human:
          vehicle_label_.text = level.to_human_cstr();
          break;
        case LabelConfiguration::Symbol:
          vehicle_label_.text = level.to_symbol_cstr();
          break;
        case LabelConfiguration::Unicode:
          vehicle_label_.text = level.to_unicode_cstr();
          break;
        default:
          throw std::runtime_error("VtdVehicle: unknown label state");
      }
      this->send_label(tx);
    }
  }

  /**
   * Send the vehicle label to VTD.
   */
  void send_label(ScpTransceiver& tx) const { tx.send(vehicle_label_); }

  /**
   * Reset the vehicle, as far as possible.
   */
  void reset() {
    for (auto kv : sensors_) {
      auto sensor = kv.second;
      sensor->reset();
    }
    this->ego_control_->reset();
    vehicle_label_.text = "!";
  }

  /**
   * Write the JSON representation into j.
   */
  friend void to_json(cloe::Json& j, const VtdVehicle& v) {
    to_json(j, dynamic_cast<const cloe::Vehicle&>(v));
    j["vtd_name"] = v.vtd_name_;
    j["sensors"] = v.sensors_;
    j["actuator"] = v.ego_control_;
  }

  void configure_components(const std::map<std::string, VtdComponentConfig>& components) {
    for (const auto& comp : components) {
      auto name = comp.first;
      auto cfg = comp.second;
      if (cfg.from.empty()) {
        // Configure actuators.
        if (cfg.type == "ego_state_model") {
          auto ego_model = std::make_shared<VtdExternalEgoModel>(task_control_, id_, vtd_name_);
          // Since no default component of this type was instantiated in the constructor, cfg.override does not apply.
          this->add_component(ego_model, name);
          ego_control_ = ego_model;
        } else {
          throw cloe::ModelError("unknown actuator component type '{}'", cfg.type);
        }
      } else {
        // Configure sensors.
        std::shared_ptr<VtdSensorData> sensor;
        if (cfg.from != "task_control") {
          if (!sensors_.count(cfg.from)) {
            throw cloe::ModelError("reference to unknown sensor '{}'", cfg.from);
          }
          sensor = this->sensors_.at(cfg.from);
        }
        auto new_or_override_component = [this](cloe::Component* c, std::string name,
                                                bool override) {
          if (override) {
            this->emplace_component(std::shared_ptr<cloe::Component>(c), name);
          } else {
            this->add_component(std::shared_ptr<cloe::Component>(c), name);
          }
        };
        if (cfg.type == "lane_sensor") {
          new_or_override_component(new VtdLaneBoundarySensor{sensor}, name, cfg.override);
        } else if (cfg.type == "object_sensor") {
          new_or_override_component(new VtdWorldSensor{sensor}, name, cfg.override);
        } else if (cfg.type == "driver_request") {
          new_or_override_component(new VtdDriverRequest{id_, task_control_}, name, cfg.override);
        } else {
          throw cloe::ModelError("unknown sensor component type '{}'", cfg.type);
        }
      }
    }
  }

 public:
  const std::string DEFAULT_SENSOR_NAME = "cloe::vtd::sensor::default";
  std::string vtd_name_{};
  uint16_t sensor_port_{0};
  uint16_t id_{0};
  std::shared_ptr<TaskControl> task_control_{nullptr};
  std::map<std::string, std::shared_ptr<VtdSensorData>> sensors_;
  std::shared_ptr<VtdVehicleControl> ego_control_{nullptr};
  scp::LabelVehicle vehicle_label_;
};

/**
 * The VtdVehicleFactory creates vehicles for VTD, taking any sensor
 * configuration into account.
 *
 * In VTD vehicles are defined with sensors bolted on afterwards. While it is
 * possible to have a default sensor configuration, in principle we need to
 * configure it ourselves via SCP messages. This is quite an involved process,
 * and so it makes sense to make this external to a vehicle.
 */
class VtdVehicleFactory {
 public:
  VtdVehicleFactory(const cloe::utility::TcpTransceiverConfiguration& c, std::string host,
                    uint16_t initial_port, const std::map<std::string, VtdVehicleConfig>& vehicles)
      : rdb_factory_(c)
      , osi_factory_(c)
      , sensor_host_(std::move(host))
      , sensor_port_(initial_port)
      , vehicles_(vehicles) {
    // store VTD vehicle names from config to track accidental leftovers
    remaining_vehicles_.resize(vehicles.size());
    std::transform(vehicles.begin(), vehicles.end(), remaining_vehicles_.begin(),
                   [](auto kv) { return kv.first; });
  }

  void set_task_control(std::shared_ptr<TaskControl> tc) { task_control_ = tc; }

  std::shared_ptr<VtdVehicle> create_or_throw(ScpTransceiver& tx, int id, const std::string& name,
                                              cloe::AbortFlag& sig) {
    assert(task_control_);
    assert(sensor_port_ != 0);
    remaining_vehicles_.erase(
        std::remove(remaining_vehicles_.begin(), remaining_vehicles_.end(), name),
        remaining_vehicles_.end());

    // Inform VTD what kind of sensors we want to be configured for our
    // vehicle.
    auto port = sensor_port_++;
    this->send_sensor_configuration(tx, port, id);

    // Give the OS a chance to let VTD open a port.
    std::this_thread::sleep_for(cloe::Milliseconds{100});

    // Connect to it, possibly retrying if necessary.
    std::unique_ptr<RdbTransceiverTcp> rdb_client{
        rdb_factory_.create_or_throw(sensor_host_, port, sig)};

    // Put it all together in form of a vehicle.
    std::shared_ptr<VtdVehicle> veh =
        std::make_shared<VtdVehicle>(id, name, std::move(rdb_client), task_control_);

    // Create and register additional configured sensors
    if (vehicles_.count(name)) {
      VtdVehicleConfig vcfg = vehicles_.at(name);
      for (auto sens : vcfg.sensors) {
        auto name = sens.first;
        auto cfg = sens.second;
        auto port = sensor_port_++;
        cloe::Json j{
            {"sensor_id", port}, {"sensor_name", name}, {"sensor_port", port}, {"player_id", id}};
        send_sensor_configuration(tx, cfg.xml, j);
        std::this_thread::sleep_for(cloe::Milliseconds(100));
        switch (cfg.protocol) {
          case ProtocolConfiguration::Rdb: {
            sensors_logger()->debug("Opening RDB channel {} for sensor {}", port, name);
            std::shared_ptr<VtdOmniSensor> omni(
                new VtdOmniSensor(rdb_factory_.create_or_throw(sensor_host_, port), id));
            veh->sensors_[name] = omni;
            break;
          }
          case ProtocolConfiguration::Osi: {
            sensors_logger()->debug("Opening TCP channel {} for OSI sensor {}", port, name);
            std::shared_ptr<VtdOsiSensor> osi(
                new VtdOsiSensor(osi_factory_.create_or_throw(sensor_host_, port), id));
            osi->configure(cfg);
            veh->sensors_[name] = osi;
            break;
          }
          default: {
            throw cloe::Error("VtdVehicle: unknown sensor protocol");
          }
        }
      }
      veh->configure_components(vcfg.components);
    }

    return veh;
  }

  std::vector<std::string> unregistered_vehicles() { return remaining_vehicles_; }

 private:
  /**
   * Send the sensor configuration of the vehicle to VTD.
   */
  void send_sensor_configuration(ScpTransceiver& tx, uint16_t sensor_port, int player_id) const {
    scp::SensorConfiguration cfg;
    cfg.port = sensor_port;
    cfg.player_id = player_id;
    cfg.sensor_id = sensor_port;
    tx.send(cfg);
  }

  /**
   * Send the sensor configuration of the vehicle to VTD.
   */
  void send_sensor_configuration(ScpTransceiver& tx, const std::string& xml,
                                 const cloe::Json& params) const {
    tx.send(cloe::utility::inja_env().render(xml, params));
  }

 private:
  std::vector<std::string> remaining_vehicles_;
  std::shared_ptr<TaskControl> task_control_{nullptr};
  RdbTransceiverTcpFactory rdb_factory_{};
  cloeosi::OsiTransceiverTcpFactory osi_factory_{};
  std::string sensor_host_{"localhost"};
  uint16_t sensor_port_{0};
  std::map<std::string, VtdVehicleConfig> vehicles_;
};

}  // namespace vtd
