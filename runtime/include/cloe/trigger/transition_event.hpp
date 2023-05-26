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
 * \file cloe/trigger/transition_event.hpp
 * \see  cloe/trigger/transition_event.cpp
 *
 * This file defines the Transition event and the corresponding
 * TransitionFactory and TransitionCallback.
 *
 * These can be used to trigger an event when a certain transition occurs,
 * for example from one enum value to another.
 *
 * For example, let us say that our model would like to provide users a way
 * to trigger when certain changes in the ACC state machine occur. Let us
 * assume that the state enum contains values:
 *
 *    enum AccState {
 *      ...
 *      ACC_ACTIVE = 1,
 *      ACC_OVERRIDE = 3
 *    };
 *
 *    void to_json(Json& j, const AccState& s);
 *    void from_json(const Json& j, AccState& s);
 *
 * These could be exposed to the user as strings or as integers. The user
 * should then be able to insert a trigger with the event corresponding to:
 *
 *    ACC_ACTIVE transitions to TARGET_SET_SPEED
 *    {"name": "acc_state", "from": 1, "to": 3}
 *    "acc_state=1->3"
 *
 * The model will need to add the TransitionCallback to its class:
 *
 *    std::shared_ptr<events::TransitionCallback<AccState>> callback_set_speed_;
 *
 * And in it's enroll(Registrar& r) method, it should register the callback:
 *
 *    callback_set_speed_ = register_event<events::Transition<AccState>, AccState>(
 *      r, std::make_unique<events::TransitionFactory<AccState>>(
 *           name() + "/acc_state", "ACC state transitions (int)"));
 *
 * In the process(const Sync& s) method, the trigger can then be called:
 *
 *    callback_set_speed_->trigger(s, acc_state_);
 *
 */

#pragma once

#include <string>  // for string

#include <cloe/registrar.hpp>  // for DirectCallback
#include <cloe/trigger.hpp>    // for Event, EventFactory

namespace cloe {
namespace events {

template <typename T>
class Transition : public Event {
 public:
  Transition(const std::string& name, T from, T to)
      : Event(name), from_(from), to_(to), ready_(false) {}
  Transition(const Transition<T>&) = delete;
  Transition(Transition<T>&&) = delete;
  ~Transition() = default;

  EventPtr clone() const override { return std::make_unique<Transition<T>>(name(), from_, to_); }

  bool operator()(const Sync&, T x) {
    if (ready_) {
      // Previous state: from
      if (x == to_) {
        // State change: from -> to
        ready_ = false;
        return true;
      } else if (x != from_) {
        // State change: from -> !from
        ready_ = false;
      }
      return false;
    } else {
      // Previous state: !from
      if (x == from_) {
        ready_ = true;
      }
      return false;
    }
  }

  void to_json(Json& j) const override {
    j = Json{
        {"from", from_},
        {"to", to_},
    };
  }

 private:
  bool ready_;
  T from_;
  T to_;
};

template <typename T>
class TransitionFactory : public EventFactory {
 public:
  using EventType = Transition<T>;
  TransitionFactory(const std::string& name, const std::string& desc) : EventFactory(name, desc) {}

  TriggerSchema schema() const override {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    static const char* desc = "transition between one state and another";
    return TriggerSchema{
        name(),
        description(),
        InlineSchema(desc, "transition", true),
        {
            {"from", make_prototype<T>().description("from state").require()},
            {"to", make_prototype<T>().description("destination state").require()},
        },
    };
  }

  EventPtr make(const Conf& c) const override {
    try {
      auto from = c.get<T>("from");
      auto to = c.get<T>("to");
      return std::make_unique<Transition<T>>(name(), from, to);
    } catch (std::exception& e) {
      throw TriggerInvalid(c, e.what());
    }
  }

  EventPtr make(const std::string& s) const override {
    auto sep = s.find("->");
    if (sep == std::string::npos) {
      throw TriggerInvalid(Conf{Json{s}}, "expected format N->M");
    }
    // Unfortunately, there's no nice way to do this in C++:
    auto from = Json{s.substr(0, sep)}.get<T>();
    auto to = Json{s.substr(sep + 2)}.get<T>();
    return make(Conf{Json{
        {"from", from},
        {"to", to},
    }});
  }
};

template <typename T>
using TransitionCallback = DirectCallback<Transition<T>, T>;

}  // namespace events
}  // namespace cloe
