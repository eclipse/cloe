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
 * \file virtue.cpp
 */

#include <map>      // for map<>
#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <cloe/component/ego_sensor.hpp>                // for EgoSensor
#include <cloe/component/object_sensor.hpp>             // for ObjectSensor
#include <cloe/component/utility/ego_sensor_canon.hpp>  // for EgoSensorCanon
#include <cloe/controller.hpp>                          // for Controller, ControllerFactory, ...
#include <cloe/models.hpp>                              // for CloeComponent
#include <cloe/plugin.hpp>                              // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                           // for DirectCallback
#include <cloe/sync.hpp>                                // for Sync
#include <cloe/trigger/macros.hpp>                      // for DEFINE_NIL_EVENT
#include <cloe/vehicle.hpp>                             // for Vehicle

namespace cloe {
namespace controller {

namespace events {

DEFINE_NIL_EVENT(Failure, "failure", "assertion failure in simulation")
DEFINE_NIL_EVENT(Collision, "collision", "collision detected in simulation")

}  // namespace events

class Failure {
 public:
  Failure(const Sync& s, std::string&& name, Json&& j)
      : name_(std::move(name)), time_(s.time()), data_(std::move(j)) {}
  virtual ~Failure() noexcept = default;

  std::string name() const { return name_; }
  Duration time() const { return time_; }
  Json data() const { return data_; }

  friend void to_json(Json& j, const Failure& f) {
    j = Json{
        {"name", f.name_},
        {"time", f.time_},
        {"data", f.data_},
    };
  }

 protected:
  std::string name_;
  Duration time_;
  Json data_;
};

using FailurePtr = std::unique_ptr<Failure>;

class Checker : public Entity, public Confable {
 public:
  explicit Checker(const std::string& name) : Entity("checker/" + name) {}
  virtual ~Checker() noexcept = default;

  void set_fail_callback(std::function<void(const Sync& s)> f) { failure_callback_ = f; }

  size_t num_failures() const { return num_failures_; }

  void fail(const Sync& s, std::string&& name, Json&& j) {
    num_failures_ += 1;
    j["sync_state"] = s;
    logger()->warn("Check failed: {}: {}", name, j.dump(2));
    failures_[name].emplace_back(std::make_unique<Failure>(s, std::move(name), std::move(j)));
    if (failure_callback_) {
      failure_callback_(s);
    }
  }

  virtual void init(const Sync&, const Vehicle&) {}

  virtual void check(const Sync& s, const Vehicle& v) = 0;

 protected:
  size_t num_failures_{0};
  std::map<std::string, std::vector<FailurePtr>> failures_{};
  std::function<void(const Sync& s)> failure_callback_;
};

using CheckerPtr = std::unique_ptr<Checker>;

class RationalityChecker : public Checker {
 public:
  RationalityChecker() : Checker("rationality") {}

  void init(const Sync&, const Vehicle& v) override {
    auto ego =
        utility::EgoSensorCanon(v.get<const EgoSensor>(CloeComponent::GROUNDTRUTH_EGO_SENSOR));

    // Test 1
    original_ego_ = ego.sensed_state();
  }

  void check(const Sync& s, const Vehicle& v) override {
    auto ego =
        utility::EgoSensorCanon(v.get<const EgoSensor>(CloeComponent::GROUNDTRUTH_EGO_SENSOR));

    // Test 1: ego object cannot change size
    auto ego_state = ego.sensed_state();
    if (ego_state.dimensions != original_ego_.dimensions) {
      fail(s, "discontinuity",
           Json{
               {"original_ego_dimensions", original_ego_.dimensions},
               {"current_ego_dimensions", ego_state.dimensions},
           });
    }

    // Test 2: normed velocity cannot be negative
    if (ego.velocity_as_mps() < 0.0) {
      fail(s, "negative_velocity",
           Json{
               {"velocity_mps", ego.velocity_as_mps()},
           });
    }
  }

 private:
  Object original_ego_;
};

class SafetyChecker : public Checker {
 public:
  SafetyChecker() : Checker("safety") {}

  void init(const Sync& s, const Vehicle& v) override {
    auto ego =
        utility::EgoSensorCanon(v.get<const EgoSensor>(CloeComponent::GROUNDTRUTH_EGO_SENSOR));

    // Test 2
    prev_mps_ = ego.velocity_as_mps();
    prev_step_ = s.step();
  }

  void check(const Sync& s, const Vehicle& v) override {
    auto ego =
        utility::EgoSensorCanon(v.get<const EgoSensor>(CloeComponent::GROUNDTRUTH_EGO_SENSOR));

    // Test 1: reported acceleration is not over max
    auto mpss = std::fabs(ego.acceleration_as_mpss());
    if (mpss > max_abs_acceleration_) {
      fail(s, "excessive_acceleration",
           Json{
               {"cur_abs_acceleration", mpss},
               {"max_abs_acceleration", max_abs_acceleration_},
           });
    }

    // Test 2: derived acceleration is not over max
    auto mps = std::fabs(ego.velocity_as_mps());
    if (std::fabs(prev_mps_ - mps) > max_abs_delta_mps(s)) {
      fail(s, "excessive_delta_velocity",
           Json{
               {"cur_delta_velocity", mps},
               {"prev_delta_velocity", prev_mps_},
               {"max_delta_velocity", max_abs_delta_mps(s)},
           });
    }
    prev_mps_ = mps;

    // Test 3: step is one-by-one
    if (s.step() != prev_step_ + 1) {
      fail(s, "discontinuous_step",
           Json{
               {"current_sync_step", s.step()},
               {"expected_sync_step", prev_step_ + 1},
           });
    }
    prev_step_ = s.step();
  }

  /**
   * Return the maximum absolute change in mps from one step to another.
   */
  double max_abs_delta_mps(const Sync& s) const {
    return max_abs_acceleration_ * (s.step_width().count() / 1'000'000'000.0);
  }

 protected:
  CONFABLE_SCHEMA(SafetyChecker) {
    return Schema{
        {"max_abs_acceleration", Schema(&max_abs_acceleration_, "max expected acceleration [m/s]")},
    };
  }

 private:
  /// The maximum plausible acceleration we should experience is 20 m/s^2.
  /// (Consider that the maximum braking achievable with tires is ~13 m/s^2,
  /// and most vehicles can't achieve more than 9 m/s^2.)
  double max_abs_acceleration_{20.0};

  // State:
  double prev_mps_{0.0};
  size_t prev_step_{0};
};

struct VirtueConfiguration : public Confable {
  Duration init_phase{100'000'000};  // should be 100ms

  CONFABLE_SCHEMA(VirtueConfiguration) {
    return Schema{
        {"init_phase", Schema(&init_phase, "time during which initialization is performed")},
    };
  }
};

class Virtue : public Controller {
 public:
  Virtue(const std::string& name, const VirtueConfiguration& c) : Controller(name), config_(c) {
    checkers_.emplace_back(std::make_unique<RationalityChecker>());
    checkers_.emplace_back(std::make_unique<SafetyChecker>());

    auto f = [this](const Sync& s) { callback_failure_->trigger(s); };
    for (auto& c : checkers_) {
      c->set_fail_callback(f);
    }
  }
  virtual ~Virtue() noexcept = default;

  void abort() override {
    // Nothing to do here.
  }

  void enroll(Registrar& r) override {
    callback_failure_ = r.register_event<events::FailureFactory>();
  }

  void start(const Sync& sync) override {
    Controller::start(sync);
    for (auto& c : checkers_) {
      c->init(sync, *veh_);
    }
  }

  Duration process(const Sync& sync) override {
    if (sync.time() < config_.init_phase) {
      start(sync);
    } else {
      for (auto& c : checkers_) {
        c->check(sync, *veh_);
      }
    }
    return sync.time();
  }

 protected:
  VirtueConfiguration config_;
  std::vector<CheckerPtr> checkers_{};
  std::shared_ptr<events::FailureCallback> callback_failure_;
};

DEFINE_CONTROLLER_FACTORY(VirtueFactory, VirtueConfiguration, "virtue",
                          "performs various quality assurance measures")

DEFINE_CONTROLLER_FACTORY_MAKE(VirtueFactory, Virtue)

}  // namespace controller
}  // namespace cloe

// Register factory as plugin entrypoint:
EXPORT_CLOE_PLUGIN(cloe::controller::VirtueFactory)
