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
 * \file basic.cpp
 * \see  basic.hpp
 */

#include "basic.hpp"

#include <chrono>   // for duration<>
#include <memory>   // for shared_ptr<>, unique_ptr<>
#include <string>   // for string
#include <tuple>    // for tie
#include <utility>  // for pair, make_pair
#include <vector>   // for vector<>

#include <boost/optional.hpp>               // for optional<>
#include <fable/schema.hpp>                 // for Schema
#include <fable/schema/boost_optional.hpp>  // for Optional<>

#include <cloe/component/driver_request.hpp>            // for DriverRequest
#include <cloe/component/latlong_actuator.hpp>          // for LatLongActuator
#include <cloe/component/object_sensor.hpp>             // for ObjectSensor
#include <cloe/component/utility/ego_sensor_canon.hpp>  // for EgoSensor, EgoSensorCanon
#include <cloe/controller.hpp>                          // for Controller, Json, etc.
#include <cloe/handler.hpp>                             // for ToJson, FromConf
#include <cloe/models.hpp>                              // for CloeComponent
#include <cloe/plugin.hpp>                              // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                           // for Registrar
#include <cloe/sync.hpp>                                // for Sync
#include <cloe/trigger.hpp>                             // for Trigger, Action, ActionFactory
#include <cloe/utility/resource_handler.hpp>            // for INCLUDE_RESOURCE
#include <cloe/vehicle.hpp>                             // for Vehicle

#include "hmi_contact.hpp"  // for PushButton, Switch

INCLUDE_RESOURCE(controller_ui, PROJECT_SOURCE_DIR "/ui/dyn_controller_ui.json");

EXPORT_CLOE_PLUGIN(cloe::controller::basic::BasicFactory)

namespace cloe {
namespace controller {
namespace basic {

namespace distance {

const double VEHICLE_LENGTH = 5.0;  // in [m]

using Algorithm = std::function<double(const utility::EgoSensorCanon& ego)>;

double safe(const utility::EgoSensorCanon& ego) {
  return std::max(ego.velocity_as_kmph(), VEHICLE_LENGTH);
}

double normal(const utility::EgoSensorCanon& ego) {
  return std::max(ego.velocity_as_kmph() / 2.0, VEHICLE_LENGTH);
}

double fifty(const utility::EgoSensorCanon&) { return 50.0; }

double crazy(const utility::EgoSensorCanon&) { return VEHICLE_LENGTH; }

const std::vector<std::pair<std::string, Algorithm>> ALGORITHMS{
    {"safe", safe},
    {"normal", normal},
    {"fifty", fifty},
    {"crazy", crazy},
};

}  // namespace distance

double get_driver_request_acceleration(std::shared_ptr<DriverRequest> driver) {
  if (!driver->has_acceleration()) {
    throw cloe::Error("basic controller: {} has no acceleration data.", driver->name());
  }
  return *driver->acceleration();
}

double get_driver_request_steering_angle(std::shared_ptr<DriverRequest> driver) {
  if (!driver->has_steering_angle()) {
    throw cloe::Error("basic controller: {} has no steering_angle data.", driver->name());
  }
  return *driver->steering_angle();
}

struct AdaptiveCruiseControl {
  AccConfiguration config;
  std::shared_ptr<Vehicle> vehicle{nullptr};

  bool enabled{false};                     // whether the function can be activated
  bool active{false};                      // whether the function is currently active
  size_t distance_algorithm{0};            // index of target distance algorithm
  boost::optional<double> target_speed{};  // target speed in [km/h]

 public:
  explicit AdaptiveCruiseControl(const AccConfiguration& c) : config(c) {}

  /**
   * Add HMI buttons to the given ContactMap.
   *
   * The HMI semantic is as follows:
   *
   * ENABLE [enabled]
   *   When toggled to true, resets target_speed and active to false.
   *   When false, none of the other HMI elements are respected, the following
   *   descriptions therefore assume that enabled is true.
   *
   * CANCEL [active]
   *   Sets active to false.
   *
   * RESUME [active, target_speed]
   *   Sets active to true; sets target_speed to the current vehicle speed
   *   only if it was previously unset.
   *
   * PLUS [target_speed]
   *   Rounds target_speed up to the nearest ten. If target_speed is unset,
   *   it uses the current vehicle speed.
   *
   * MINUS [target_speed]
   *   Rounds the target_speed down to the nearest ten. If target_speed is
   *   unset, it uses the current vehicle speed as it's initial state.
   *
   * DISTANCE [distance_algorithm]
   *   Toggles the distance_algorithm between the available distance algorithms:
   *   SAFE
   *     Keeps vehicle.norm_speed in distance, minimum vehicle length.
   *   NORMAL
   *     Keeps vehicle.norm_speed / 2 in distance, minimum vehicle length.
   *   FIFTY
   *     Keeps 50 m distance.
   *   CRAZY
   *     Keeps a vehicle length from the front car.
   */
  void add_hmi(utility::ContactMap<Duration>& hmi, const std::string& prefix = "") {
    namespace contact = utility::contact;
    using PushButton = utility::PushButton<Duration>;

    auto restore_target_speed = [](AdaptiveCruiseControl* acc) {
      assert(acc->vehicle != nullptr);
      if (!acc->target_speed) {
        auto ego = utility::EgoSensorCanon(acc->vehicle->get<EgoSensor>(acc->config.ego_sensor));
        acc->target_speed = ego.velocity_as_kmph();
      }
    };

    hmi.add_new(prefix + "enable",
                new utility::Switch<Duration>(
                    [this]() {
                      // Switch set to ON
                      this->enabled = true;
                      this->active = false;
                      this->target_speed.reset();
                    },
                    [this]() {
                      // Switch set to OFF
                      this->enabled = false;
                    },
                    this->enabled));
    hmi.add_new(prefix + "cancel", new PushButton([this]() { this->active = false; }));
    hmi.add_new(prefix + "resume", new PushButton([&, this]() {
                  this->active = true;
                  restore_target_speed(this);
                }));
    hmi.add_new(prefix + "plus",
                new PushButton(
                    [&, this]() {
                      restore_target_speed(this);
                      *this->target_speed = contact::round_step(*this->target_speed, 10.0);
                    },
                    [&, this]() {
                      restore_target_speed(this);
                      *this->target_speed = contact::round_step(*this->target_speed, 5.0);
                    }));
    hmi.add_new(prefix + "minus",
                new PushButton(
                    [&, this]() {
                      restore_target_speed(this);
                      *this->target_speed = contact::round_step(*this->target_speed, -10.0);
                      if (*this->target_speed < 0.0) {
                        *this->target_speed = 0.0;
                      }
                    },
                    [&, this]() {
                      restore_target_speed(this);
                      *this->target_speed = contact::round_step(*this->target_speed, -5.0);
                      if (*this->target_speed < 0.0) {
                        *this->target_speed = 0.0;
                      }
                    }));
    hmi.add_new(prefix + "distance", new PushButton([this]() {
                  this->distance_algorithm =
                      (this->distance_algorithm + 1) % distance::ALGORITHMS.size();
                }));
  }

  /**
   * FIXME(ben): The HMI should not be manipulated while we are in this part.
   */
  void control(Vehicle& v, const Sync& sync, const std::string& driver_request) {
    assert(distance_algorithm < distance::ALGORITHMS.size());

    if (!enabled || !active) {
      // When not enabled, the function is disabled except for the HMI,
      // which is controlled separately.
      if (!driver_request.empty()) {
        double acc = get_driver_request_acceleration(v.get<DriverRequest>(driver_request));
        v.get<LatLongActuator>(config.latlong_actuator)->set_acceleration(acc);
      }
      return;
    }

    assert(target_speed);
    auto world_sensor = v.get<ObjectSensor>(config.world_sensor);
    auto objects = world_sensor->sensed_objects();
    auto rabbit = utility::closest_forward(objects);
    const double fac_to_switch_control{0.2};  // Factor to change the control from speed to distance
    auto ego = utility::EgoSensorCanon(v.get<EgoSensor>(config.ego_sensor));
    auto vel = ego.velocity_as_kmph();
    double deviation = *target_speed - vel;

    auto pid_control = [&](double deviation, double kp, double kd, double ki,
                           double& deviation_last, double& uki_last) {
      std::chrono::duration<double> time_step = sync.step_width();
      double ukp = kp * deviation;
      double uki = ki * (uki_last + time_step.count() * deviation);
      double ukd = kd * (deviation - deviation_last);
      uki_last = (uki_last + time_step.count() * deviation);
      deviation_last = deviation;
      return (ukp + ukd + uki);
    };
    // Implementation of the PID control to control the speed
    double acc = pid_control(deviation, config.kp, config.kd, config.ki, deviation_last, uki_last);
    if (rabbit) {
      double target_distance = get_distance_algorithm().second(ego);
      double deviation_m = utility::distance_forward(*rabbit) - target_distance;
      double rabbit_m_s = vel + rabbit->velocity.sum();  // absolute rabbit velocity
      if (deviation_m < (fac_to_switch_control * target_distance) && rabbit_m_s < *target_speed) {
        // Implementation of the PID control to control the distance
        acc = pid_control(deviation_m, config.kp_m, config.kd_m, config.ki_m, deviation_m_last,
                          uki_m_last);
      }
    }
    if (acc > config.limit_acceleration) {
      acc = config.limit_acceleration;
    }
    if (acc < -config.limit_deceleration) {
      acc = -config.limit_deceleration;
    }
    auto actuator = v.get<LatLongActuator>(config.latlong_actuator);
    actuator->set_acceleration(acc);
  }

  const std::pair<std::string, distance::Algorithm>& get_distance_algorithm() const {
    return distance::ALGORITHMS[distance_algorithm];
  }

  friend void to_json(Json& j, const AdaptiveCruiseControl& c) {
    j = Json{
        {"enabled", c.enabled},
        {"active", c.active},
        {"target_speed", c.target_speed},
        {"distance_algorithm", c.get_distance_algorithm().first},
    };
    if (c.target_speed) {
      j["target_speed_mps"] = static_cast<double>(*c.target_speed) * (1000.0 / 3600.0);
    } else {
      j["target_speed_mps"] = nullptr;
    }
  }

 protected:
  double uki_last{0.0};
  double uki_m_last{0.0};
  double deviation_last{0.0};
  double deviation_m_last{0.0};
};

struct LaneKeepingAssistant {
  LkaConfiguration config;

 public:
  explicit LaneKeepingAssistant(const LkaConfiguration& c) : config(c) {}

  void control(Vehicle& v, const Sync&, const std::string& driver_request) {
    if (!config.enabled) {
      if (!driver_request.empty()) {
        double rad = get_driver_request_steering_angle(v.get<DriverRequest>(driver_request));
        v.get<LatLongActuator>(config.latlong_actuator)->set_steering_angle(rad);
      }
      return;
    }

    auto world_sensor = v.get<ObjectSensor>(config.world_sensor);
    auto objects = world_sensor->sensed_objects();
    auto rabbit = utility::closest_forward(objects);

    if (rabbit) {
      auto d = utility::distance_starboard(*rabbit);
      double rad{0.0};
      if (fabs(d) <= config.tolerance) {
        rad = d * config.lerp_factor;
      } else {
        rad = config.adjustment_rad * d * config.lerp_factor;
      }

      auto actuator = v.get<LatLongActuator>(config.latlong_actuator);
      actuator->set_steering_angle(-rad);
    }
  }
};

struct AutoEmergencyBraking {
  AebConfiguration config;

 public:
  explicit AutoEmergencyBraking(const AebConfiguration& c) : config(c) {}

  void control(Vehicle& v, const Sync&, const std::string& driver_request) {
    if (!config.enabled) {
      if (!driver_request.empty()) {
        double acc = get_driver_request_acceleration(v.get<DriverRequest>(driver_request));
        v.get<LatLongActuator>(config.latlong_actuator)->set_acceleration(acc);
      }
      return;
    }
    auto world_sensor = v.get<ObjectSensor>(config.world_sensor);
    auto objects = world_sensor->sensed_objects();
    auto rabbit = utility::closest_forward(objects);
    auto ego = utility::EgoSensorCanon(v.get<EgoSensor>(config.ego_sensor));
    auto vel = ego.velocity_as_mps();
    auto actuator = v.get<LatLongActuator>(config.latlong_actuator);
    double acc{0.0};
    const double safety_factor{-2.5};  // safety margin factor to evaluate safe brake distance
    const double min_dist_to_front{
        5.0};  // safety margin which describes the minimal distance to the front car
    const double standstill_speed{0.05};  // defines which speed is interpreted as standstill

    if (rabbit) {
      double target_distance = utility::distance_forward(*rabbit);
      double deviation = rabbit->velocity.sum();
      double brakingDistance = safety_factor * deviation;

      if (target_distance < brakingDistance || target_distance < min_dist_to_front) {
        acc = -config.limit_deceleration;
        if (config.always_full_stop) {
          full_stop_activated = true;
        }
        actuator->set_acceleration(acc);
      }
    }
    // if braking to full_stop was triggered
    if (full_stop_activated) {
      acc = -config.limit_deceleration;
      actuator->set_acceleration(acc);
    }
    // reset the activated button (otherwise car will stand forever)
    if (vel < standstill_speed) {
      full_stop_activated = false;
    }
  }

 protected:
  bool full_stop_activated{false};
};

class BasicController : public Controller {
 public:
  BasicController(const std::string& name, const BasicConfiguration& c)
      : Controller(name), acc_(c.acc), aeb_(c.aeb), lka_(c.lka), driver_request_(c.driver_request) {
    // Define the HMI of the basic controller:
    namespace contact = utility::contact;
    acc_.add_hmi(hmi_);
    hmi_.add_new("aeb", contact::make_switch(&aeb_.config.enabled));
    hmi_.add_new("lka", contact::make_switch(&lka_.config.enabled));
  }

  virtual ~BasicController() noexcept = default;

  void abort() override {
    // Nothing to do here.
  }

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<utility::ContactFactory<Duration>>(&hmi_));

    // clang-format off
    r.register_api_handler("/configuration", HandlerType::BUFFERED, handler::ToJson<BasicController>(this));
    r.register_api_handler("/configuration/acc", HandlerType::DYNAMIC, handler::FromConf(&acc_.config));
    r.register_api_handler("/configuration/aeb", HandlerType::DYNAMIC, handler::FromConf(&aeb_.config));
    r.register_api_handler("/configuration/lka", HandlerType::DYNAMIC, handler::FromConf(&lka_.config));
    r.register_api_handler("/state", HandlerType::BUFFERED, handler::ToJson<AdaptiveCruiseControl>(&acc_));
    r.register_api_handler("/hmi", HandlerType::BUFFERED, handler::ToJson<utility::ContactMap<Duration>>(&hmi_));
    r.register_api_handler("/hmi/set", HandlerType::DYNAMIC, handler::FromConf(&hmi_));
    r.register_api_handler("/vehicle", HandlerType::BUFFERED, [this](const Request&, Response& r) {
      if (this->veh_ == nullptr) {
        r.server_error(Json{{"error", "vehicle is null"}});
      } else {
        r.write(*this->veh_);
      }
    });
    r.register_api_handler("/ui", HandlerType::STATIC, RESOURCE_HANDLER(controller_ui, ContentType::JSON));
    // clang-format on
  }

  void set_vehicle(std::shared_ptr<cloe::Vehicle> v) override {
    Controller::set_vehicle(v);
    acc_.vehicle = v;
  }

  Duration process(const Sync& sync) override {
    assert(veh_ != nullptr);

    hmi_.update(sync.time());
    acc_.control(*veh_, sync, driver_request_);
    lka_.control(*veh_, sync, driver_request_);
    aeb_.control(*veh_, sync, driver_request_);

    return sync.time();
  }

  friend void to_json(Json& j, const BasicController& c) {
    j = Json{
        {"acc", c.acc_.config},
        {"aeb", c.aeb_.config},
        {"lka", c.lka_.config},
    };
  }

 protected:
  AdaptiveCruiseControl acc_;
  AutoEmergencyBraking aeb_;
  LaneKeepingAssistant lka_;
  std::string driver_request_;
  utility::ContactMap<Duration> hmi_;
};

DEFINE_CONTROLLER_FACTORY_MAKE(BasicFactory, BasicController)

}  // namespace basic
}  // namespace controller
}  // namespace cloe
