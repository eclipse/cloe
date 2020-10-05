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
 * \file vtd_binding.cpp
 */

#include <iostream>  // for istringstream
#include <memory>    // for unique_ptr<>, shared_ptr<>
#include <numeric>   // for accumulate
#include <string>    // for string
#include <thread>    // for this_thread::sleep_for
#include <utility>   // for move
#include <vector>    // for vector<>

#include <boost/optional.hpp>                  // for optional<>
#include <boost/property_tree/ptree.hpp>       // for ptree
#include <boost/property_tree/xml_parser.hpp>  // for read_xml
#include <fable/json/with_boost.hpp>           // for to_json

#include <cloe/core/abort.hpp>          // for AbortFlag, abort_checkpoint
#include <cloe/plugin.hpp>              // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>           // for Registrar
#include <cloe/simulator.hpp>           // for Simulator, ModelError
#include <cloe/sync.hpp>                // for Sync
#include <cloe/utility/statistics.hpp>  // for Accumulator
#include <cloe/utility/timer.hpp>       // for DurationTimer
#include <cloe/vehicle.hpp>             // for Vehicle

#include "rdb_transceiver_tcp.hpp"  // for RdbTransceiverTcpFactory
#include "scp_messages.hpp"         // for ScenarioStartConfig
#include "scp_transceiver.hpp"      // for ScpTransceiver
#include "task_control.hpp"         // for TaskControl
#include "vtd_conf.hpp"             // for VtdConfiguration
#include "vtd_vehicle.hpp"          // for VtdVehicle

namespace vtd {

/**
   * The VtdStatistics struct contains all nominal statistics of the VTD binding.
   */
struct VtdStatistics {
  cloe::utility::Accumulator frame_time_ms;
  cloe::utility::Accumulator clock_drift_ns;

  /**
     * Writes the JSON representation into j.
     *
     * # JSON Output
     * ```json
     * {
     *   {"connection_tries": number},
     *   {"frame_time_ms", Accumulator},
     *   {"clock_drift_ns", Accumulator}
     * }
     * ```
     */
  friend void to_json(cloe::Json& j, const VtdStatistics& s) {
    j = cloe::Json{
        {"frame_time_ms", s.frame_time_ms},
        {"clock_drift_ns", s.clock_drift_ns},
    };
  }
};

class VtdBinding : public cloe::Simulator {
 public:
  VtdBinding() = delete;
  ~VtdBinding() = default;

  VtdBinding(const std::string& name, const VtdConfiguration& config)
      : Simulator(name)
      , config_(config)
      , vehicle_factory_(config.rdb_params, config.connection.host, config.sensor_initial_port,
                         config.vehicles) {}

  void abort() final { abort_signal_.store(true); }

  void connect() final {
    assert(!is_connected());
    logger()->info("Connecting...");

    size_t attempt = 0;
    auto handle_configure_error = [&](cloe::Error& e) {
      logger()->error("Configuration attempt {} failed: {}", attempt, e.what());
      if (attempt > config_.configuration_retry_attempts) {
        throw cloe::ModelError("cannot configure VTD");
      }

      // Otherwise, reset state for the next try.
      clear();
      disconnect();
    };

    bool success = false;
    while (!success) {
      try {
        attempt++;
        if (attempt > 1) {
          logger()->info("Connecting... [attempt {}/{}]", attempt,
                         config_.configuration_retry_attempts);
        }
        connect_and_configure(attempt);
        success = true;
      } catch (cloe::utility::TcpReadError& e) {
        handle_configure_error(e);
      } catch (ScpError& e) {
        handle_configure_error(e);
      }
    }

    logger()->info("Connected.");
    assert(is_operational());
  }

  void disconnect() final {
    logger()->info("Disconnecting...");
    for (auto& v : vehicles_) {
      v->disconnect();
    }
    vehicles_.clear();
    paramserver_client_->tcp_disconnect();
    scp_client_->tcp_disconnect();
    task_control_.reset();
    Simulator::disconnect();
    logger()->info("Disconnected.");
  }

  /**
   * Connect to and configure VTD.
   *
   * Reads SCP bus and collects VTD state information until VTD initialization
   * is completed.
   */
  void connect_and_configure(size_t /* attempt */) {
    // Try to connect to VTD.
    try {
      paramserver_client_.reset(                                                           // NOLINT
          cloe::utility::create_or_throw_with<ScpTransceiverFactory>(config_.paramserver,  // NOLINT
                                                                     abort_signal_)        // NOLINT
              .release());                                                                 // NOLINT
      scp_client_.reset(cloe::utility::create_or_throw_with<ScpTransceiverFactory>(        // NOLINT
                            config_.connection, abort_signal_)                             // NOLINT
                            .release());                                                   // NOLINT
    } catch (std::ios_base::failure&) {
      throw cloe::ModelError("cannot connect to VTD");
    }

    // Try to configure VTD.
    {
      // FIXME(henning): Replace sleep with waiting for a proper handshake.
      // That requires a reliable signal from VTD that acknowledges Apply
      // being successful and done. Currently this is only indicated by
      // debug level info messages, containing taskcontrol and modulemanager
      // state indication. Debug messages are not reliable as they might be
      // configured away. Second indication available is Messages from the
      // ImageGenerator. These are not available in headless mode.
      // So currently sleeping is the only available workaround.
      //
      // NOTE: This workaround might fail in situations with heavy load when
      //       'apply' takes much longer than usual.
      scp_client_->send(scp::Config);
      scp::ParamServerConfig pc;
      pc.sync_source = "RDB";
      pc.no_image_generator = (!config_.image_generator || config_.setup == "Cloe.noGUInoIG" ||
                               config_.setup == "Cloe.noIG");

      auto sleep_awhile = [this]() {
        cloe::abort_checkpoint(abort_signal_);
        std::this_thread::sleep_for(cloe::Milliseconds{VTD_INIT_SYNC_SLEEP_MS});
      };

      sleep_awhile();
      paramserver_client_->send(pc);
      scp_client_->send(scp::Apply);

      if (config_.dat_file.size()) {
        logger()->info("Recording data file: {}", config_.dat_file);
        sleep_awhile();
        scp::RecordDat recdat;
        recdat.datfile_path = config_.dat_file;
        scp_client_->send(recdat);
      }

      if (config_.scenario.size()) {
        logger()->info("Starting scenario: {}", config_.scenario);
        sleep_awhile();
        scp::ScenarioStartConfig vtd_scenario;
        vtd_scenario.filename = config_.scenario;
        scp_client_->send(vtd_scenario);
      }
    }

    {
      logger()->info("Wait for scenario...");
      int tries_left = VTD_INIT_WAIT_RETRIES;

      while (!is_operational() || !task_control_) {
        // Expect following to happen:
        // 1. TaskControl client created
        // 2. Vehicles created and connected
        // 3. Simulator starts running

        // FIXME(ben): This could fail with a runtime error:
        //
        //   terminate called after throwing an instance of 'std::runtime_error'
        //     what():  ScpTransceiver: error during read: Broken pipe
        //
        // In this case, we probably need to reconnect to SCP and try again.
        this->readall_scp();
        std::this_thread::sleep_for(cloe::Milliseconds{VTD_INIT_WAIT_SLEEP_MS});

        // TODO(henning): Every scp message that is not ending the while loop
        //                is a missed try. Just assume VTD or anyone connected
        //                to the SCP channel to write 300 individual messages
        //                and this will fail. So we need to use a true timeout
        //                here (or elsewhere as pointed out above).
        if (!tries_left--) {
          throw cloe::ModelError("timeout waiting for SCP message");
        }
      }

      logger()->info("Scenario loaded.");
    }

    if (num_vehicles() == 0) {
      throw cloe::ModelError("no vehicles created");
    }

    std::vector<std::string> unreg_vehicles = vehicle_factory_.unregistered_vehicles();
    if (unreg_vehicles.size() != 0) {
      std::string vlist;
      vlist = std::accumulate(unreg_vehicles.begin(), unreg_vehicles.end(), std::string(),
                              [](const std::string& a, const std::string& b) {
                                return a + (a.length() ? ", " : "") + b;
                              });
      throw cloe::ModelError("VTD sensor config(s) unused. Check VTD player name(s): {}", vlist);
    }

    // Set the camera position and initial vehicle labels.
    if (config_.camera_third_person) {
      this->init_camera_position(config_.camera_focus_on);
    }
    if (config_.label_vehicle != LabelConfiguration::Off) {
      for (auto v : vehicles_) {
        v->send_label(*scp_client_);
      }
    }

    // Trigger frame 1
    task_control_->add_trigger_and_send(cloe::Duration(0));

    Simulator::connect();
  }

  void enroll(cloe::Registrar& r) final {
    r.register_api_handler("/state", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<VtdBinding>(this));
    r.register_api_handler("/configuration", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<VtdConfiguration>(&config_));
    r.register_api_handler("/statistics", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<VtdStatistics>(&stats_));
  }

  /**
   * Clear internal data-structures so that we can configure() again.
   */
  void clear() { vehicles_.clear(); }

  /**
   * Restart VTD and reset the essential parts of the binding.
   *
   * This is not a true reset as some state VTD will remain (e.g. sensor
   * configuration).  But the simulation time together with all objects of the
   * scenario will start over from 0.
   */
  void reset() final {
    // send a restart to VTD as reset request didn't come from VTD
    scp_client_->send(scp::Restart);

    // If in reset, block until VTD sends "Run" again then start next cycle
    operational_ = false;
    while (!operational_) {
      readall_scp();
    }

    // Reset all sensors in each vehicle
    for (auto v : vehicles_) {
      v->reset();
    }

    task_control_->reset();
  }

  size_t num_vehicles() const final { return vehicles_.size(); }
  std::shared_ptr<cloe::Vehicle> get_vehicle(size_t i) const final { return vehicles_[i]; }
  std::shared_ptr<cloe::Vehicle> get_vehicle(const std::string& key) const final {
    try {
      return this->get_vehicle_by_name(key);
    } catch (std::out_of_range&) {
      return nullptr;
    }
  }

  void start(const cloe::Sync&) final {
    // operational_ is set by connect() not start().
    assert(is_operational());
  }

  /**
   * Synchronize with VTD and trigger the next VTD frame calculation.
   *
   * It performs the following steps:
   * - It sends vehicle actuations.
   * - It retrieves the new world state.
   * - It updates the vehicles.
   * - It triggers VTD to perform the next frame calculation.
   */
  cloe::Duration process(const cloe::Sync& sync) final {
    // Preconditions:
    assert(task_control_);
    assert(is_connected());
    assert(is_operational());

    // Statistics:
    timer::DurationTimer<timer::Milliseconds> t(
        [this](timer::Milliseconds d) { this->stats_.frame_time_ms.push_back(d.count()); });

    // Read all incoming SCP messages,
    // a) to empty the buffer, and
    // b) to catch any restart requests.
    this->readall_scp();

    // Send items to TaskControl:
    for (auto v : vehicles_) {
      v->vtd_step_actuator(*scp_client_, config_.label_vehicle);
    }

    // Process task control messages
    task_control_->step(sync);

    // Receive new data relating to all sensors
    cloe::Duration sensor_time{0};
    for (auto v : vehicles_) {
      sensor_time = v->vtd_step_sensors(sync);
    }

    // Trigger VTD to simulation the next step
    task_control_->add_trigger_and_send(sync.step_width());

    // Calculate error of previous timestep for timing statistics
    vtd_timestep_error_ = sync.time() - sensor_time;
    stats_.clock_drift_ns.push_back(static_cast<double>(vtd_timestep_error_.count()));

    return sync.time();
  }

  void stop(const cloe::Sync&) final {
    scp_client_->send(scp::Stop);
    scp_client_->send(scp::Config);
  }

 protected:
  std::shared_ptr<VtdVehicle> get_vehicle_by_id(uint64_t id) const {
    for (auto v : vehicles_) {
      if (v->id() == id) {
        return v;
      }
    }
    return std::shared_ptr<VtdVehicle>{nullptr};
  }

  /**
   * Return a VtdVehicle with the given name, or nullptr if it doesn't exist.
   *
   * Thus, this function can be used to also check if a vehicle has a particular name:
   *
   *    if (this->get_vtd_vehicle(name)) {
   *      // ...
   *    }
   */
  std::shared_ptr<VtdVehicle> get_vehicle_by_name(const std::string& name) const {
    for (auto v : vehicles_) {
      if (v->vtd_name() == name) {
        return v;
      }
    }
    return std::shared_ptr<VtdVehicle>{nullptr};
  }

  /**
   * Set the initial camera position on the vehicle with the given name.
   */
  void init_camera_position(const std::string& name) {
    std::shared_ptr<VtdVehicle> v;
    if (name == "") {
      v = vehicles_[0];
    } else {
      v = get_vehicle_by_name(name);
    }
    scp::CameraPosition cp;
    cp.tethered_to_player = v->vtd_name_;
    cp.look_to_player = v->vtd_name_;
    scp_client_->send(cp);
  }

  /**
   * Read as many SCP messages as the client currently has in the buffer and
   * apply them, one after another.
   *
   * This can result in pretty much any change in the binding, including:
   * - Starting, stopping, resetting the binding
   * - Creating vehicles
   *
   * Note: currently there is no need to read a "single" scp message, so
   * there is no read_scp function anymore.
   */
  void readall_scp() {
    // While there are incoming SCP messages...
    while (scp_client_->has()) {
      cloe::abort_checkpoint(abort_signal_);
      std::string message = scp_client_->receive();
      this->apply_scp(message);
    }
  }

  /**
   * Parse selected VTD SCP messages and call the relevant apply methods.
   */
  void apply_scp(const std::string& scp_message) {
    boost::property_tree::ptree pt;
    std::istringstream is(scp_message);
    boost::property_tree::read_xml(is, pt);
    boost::optional<boost::property_tree::ptree&> child;
    // TODO(ben): Incorrect, we need to iterate over all children. There could be multiple!
    if ((child = pt.get_child_optional("TaskControl.RDB"))) {
      this->apply_scp_rdb(*child);
    } else if ((child = pt.get_child_optional("Set"))) {
      this->apply_scp_set(*child);
    } else if ((child = pt.get_child_optional("SimCtrl.Run"))) {
      this->apply_scp_run(*child);
    } else if ((child = pt.get_child_optional("SimCtrl.Stop"))) {
      this->apply_scp_stop(*child);
    } else if ((child = pt.get_child_optional("SimCtrl.Restart"))) {
      this->apply_scp_restart(*child);
    }
  }

  void apply_scp_rdb(boost::property_tree::ptree& xml) {
    if (!xml.get("<xmlattr>.enable", false)) {
      throw cloe::ModelError("RDB not activated in VTD configuration");
    }
    auto tc_port = xml.get<uint16_t>("<xmlattr>.portRx", 0);
    if (tc_port == 0) {
      // No task control port, so nothing to do here.
      return;
    }
    auto rdb_factory = RdbTransceiverTcpFactory(config_.task_control_params);
    task_control_.reset(
        new TaskControl(rdb_factory.create_or_throw(config_.connection.host, tc_port)));
    task_control_->set_name("task_control");
    vehicle_factory_.set_task_control(task_control_);
  }

  /**
   * Create a new vehicle if one with the given ID does not already exist.
   *
   * We get this before "SimCtrl.Run" arrives.
   * However, users may use the Set command, and therefore these are only
   * processed while VTD is not running.
   */
  void apply_scp_set(boost::property_tree::ptree& xml) {
    if (operational_) {
      logger()->debug("Ignoring SCP <Set> command, because VTD is running.");
      return;
    }

    if (!task_control_) {
      logger()->warn("Cannot apply SCP <Set> command with task control client.");
      return;
    }

    int id = xml.get("<xmlattr>.id", -1);
    if (id == -1) {
      logger()->debug("Cannot apply SCP <Set> command without an ID attribute.");
      return;
    }

    if (this->get_vehicle_by_id(id)) {
      logger()->warn("Cannot apply SCP <Set> command because vehicle with ID already exists.");
      return;
    }

    auto name = xml.get("<xmlattr>.name", "default");
    auto veh = vehicle_factory_.create_or_throw(*scp_client_, id, std::move(name), abort_signal_);
    logger()->info("Agent vehicle {} with id {} was created", name, id);
    if (config_.label_vehicle != LabelConfiguration::Off) {
      veh->send_label(*scp_client_);
    }
    vehicles_.push_back(veh);
  }

  void apply_scp_run(boost::property_tree::ptree&) { operational_ = true; }

  void apply_scp_restart(boost::property_tree::ptree&) {
    // Consume SCP Restart only if it has been sent by our reset() function.
    // Otherwise throw a runtime error because it means that someone externally
    // and asynchronously sent the SCP Restart command.
    // The latter would induce a race condition because we can't decide whether
    // we've sent our last time trigger to VTD before or after the reset i.e.
    // whether VTD is at t=0 or at t=1 when Cloe is reset to t=0.
    //
    // Note: This check might fail if an internal and external restart request
    //       coincide and thus operational_ is set to false. The resulting
    //       duplicate SCP Restart without waiting for SCP Run may lead to
    //       undefined behavior or further race conditions.
    if (operational_) {
      throw cloe::ModelError("third party restarted VTD via SCP");
    }
  }

  void apply_scp_stop(boost::property_tree::ptree&) { operational_ = false; }

 private:
  VtdConfiguration config_{};

  /**
   * The vehicle factory has most everything required for creating vehicles.
   */
  VtdVehicleFactory vehicle_factory_;

  /**
   * SCP client for configuring the parameter server
   *
   * \see  connect method
   */
  std::unique_ptr<ScpTransceiver> paramserver_client_{};

  /**
   * SCP client for receiving and sending messages.
   *
   * \see  connect method
   */
  std::unique_ptr<ScpTransceiver> scp_client_{};

  /**
   * Task Control client, set by incoming SCP message "<TaskControl>".
   * This connection is only used for sending messages.
   *
   * \see  apply_scp_rdb method
   */
  std::shared_ptr<TaskControl> task_control_{};

  /// Stores all vehicles filled with the appropriate sensors and actuators.
  std::vector<std::shared_ptr<VtdVehicle>> vehicles_{};

  /// Contains statistics published via the web server.
  VtdStatistics stats_;

  /// The last triggered delta time (timestep).
  cloe::Duration vtd_timestep_pending_{0};

  /**
   * Error of last timestep.
   *
   * Can't be applied until the next trigger as we get the current timestamp
   * only after having triggered the next one.
   */
  cloe::Duration vtd_timestep_error_{0};

  cloe::AbortFlag abort_signal_{false};

  friend void to_json(cloe::Json& j, const VtdBinding& b) {
    j = cloe::Json{
        {"paramserver_connection", b.paramserver_client_},
        {"scp_connection", b.scp_client_},
        {"task_control_connection", b.task_control_},
        {"is_connected", b.connected_},
        {"is_operational", b.operational_},
        {"num_vehicles", b.num_vehicles()},
        {"vehicles", b.vehicles_},
    };
  }
};

DEFINE_SIMULATOR_FACTORY(VtdFactory, VtdConfiguration, "vtd", "VIRES Virtual Test Drive")
DEFINE_SIMULATOR_FACTORY_MAKE(VtdFactory, VtdBinding)

}  // namespace vtd

EXPORT_CLOE_PLUGIN(vtd::VtdFactory)
