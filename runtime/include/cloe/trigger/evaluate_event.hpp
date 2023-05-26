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
 * \file cloe/trigger/evaluate_event.hpp
 * \see  cloe/trigger/evaluate_event.cpp
 *
 * This file defines the Evaluate event and the corresponding EvaluateFactory.
 * These can be used to make comparisons between two values. For example,
 * let us say that our model would like to make a comparison between some
 * internal value SET_SPEED and a desired value TARGET_SET_SPEED. The user
 * should then be able to insert a trigger with the event corresponding to:
 *
 *    SET_SPEED >= TARGET_SET_SPEED
 *
 * The model will need to add the EvaluateCallback to its class:
 *
 *    std::shared_ptr<events::EvaluateCallback> callback_set_speed_;
 *
 * And in it's enroll(Registrar& r) method, it should register the callback:
 *
 *    callback_set_speed_ = register_event<events::Evaluate, double>(
 *      r, std::make_unique<events::EvaluateFactory>(
 *           name() + "/set_speed", "set speed in km/h"));
 *
 * In the process(const Sync& s) method, the trigger can then be called:
 *
 *    callback_set_speed_->trigger(s, set_speed_);
 *
 */

#pragma once

#include <functional>  // for function<>
#include <string>      // for string

#include <cloe/registrar.hpp>  // for DirectCallback
#include <cloe/trigger.hpp>    // for Event, EventFactory

namespace cloe {
namespace events {

class Evaluate : public Event {
 public:
  Evaluate(const std::string& name, const std::string& repr, std::function<bool(double)> f)
      : Event(name), repr_(repr), func_(f) {}
  EventPtr clone() const override { return std::make_unique<Evaluate>(name(), repr_, func_); }
  bool operator()(const Sync&, double d);
  void to_json(Json& j) const override;

 private:
  std::string repr_;
  std::function<bool(double)> func_;
};

class EvaluateFactory : public EventFactory {
 public:
  using EventType = Evaluate;
  EvaluateFactory(const std::string& name, const std::string& desc) : EventFactory(name, desc) {}
  TriggerSchema schema() const override;
  EventPtr make(const Conf& c) const override;
  EventPtr make(const std::string& s) const override;
};

using EvaluateCallback = DirectCallback<Evaluate, double>;

}  // namespace events
}  // namespace cloe
