/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file cloe/component/vehicle_state_model.hpp
 */

#pragma once

#include <functional>  // for function

#include <boost/optional.hpp>                // for optional<>
#include <fable/json.hpp>                    // for Json
#include <fable/utility/boost_optional.hpp>  // for to_json

#include <cloe/component.hpp>         // for Component
#include <cloe/component/object.hpp>  // for Object

namespace cloe {

class VehicleStateModel : public Component {
 public:
  using Component::Component;
  VehicleStateModel() : Component("vehicle_state") {}
  virtual ~VehicleStateModel() noexcept = default;

  /**
   * Set the ego vehicle state corresponding to the end of the current step.
   */
  virtual void set_vehicle_state(const Object& obj) { vehicle_state_ = obj; }

  /**
   * Get the ego vehicle state at the end of the current step. Note that
   * vehicle_state() may invoke set_vehicle_state() by calling
   * vehicle_state_callback_().
   */
  const boost::optional<Object>& vehicle_state() {
    if (vehicle_state_callback_) {
      vehicle_state_callback_();
    }
    return vehicle_state_;
  }

  /**
   * Return true if set_vehicle_state was called for the current step. Note that
   * is_vehicle_state() may invoke set_vehicle_state() by calling
   * vehicle_state_callback_().
   */
  bool is_vehicle_state() {
    if (vehicle_state_callback_) {
      vehicle_state_callback_();
    }
    return static_cast<bool>(vehicle_state_);
  }

  /**
   * Register callback function that will invoke set_vehicle_state().
   */
  void register_vehicle_state_callback(const std::function<void(void)>& c) {
    vehicle_state_callback_ = c;
  }

  /**
   * Write the JSON representation of the vehicle state into j.
   *
   * Currently, the API is unstable, because we don't have access to any real
   * data.
   */
  fable::Json active_state() const override {
    return fable::Json{
        {"vehicle_state", vehicle_state_},
    };
  }

  Duration process(const Sync& sync) override {
    auto t = Component::process(sync);
    vehicle_state_.reset();
    return t;
  }

  void reset() override {
    Component::reset();
    vehicle_state_.reset();
  }

 protected:
  /**
   * Vehicle state determined by a vehicle dynamics model.
   *
   * Contains object pose, velocity, acceleration, angular_velocity in world coordinates.
   */
  boost::optional<Object> vehicle_state_;

  /**
   * Callback function that is invoked when access to the ego vehicle target
   * state is requested (by calling is_vehicle_state() or vehicle_state()).
   *
   * The main use case for the callback is to update the ego vehicle state using
   * an actuator and/or vehicle dynamics model external of the simulator. Then,
   * the callback could implement the following:
   *  - Update the external model with the latest simulator state
   *  - Trigger the time stepping of the external model
   *  - Invoke set_vehicle_state() to update LatLongActuator
   *
   * Note that the callback function must ensure that repeated invocation within
   * the same time step does not lead to unintended behavior.
   */
  std::function<void(void)> vehicle_state_callback_;
};

}  // namespace cloe
