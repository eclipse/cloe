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
 * \file time_event.hpp
 *
 * This file defines the "time" and "next" events.
 */

#pragma once

#include <chrono>  // for duration_cast<>
#include <memory>  // for unique_ptr<>, make_unique<>
#include <queue>   // for priority_queue<>

#include <cloe/core.hpp>     // for Json, Duration, Seconds
#include <cloe/sync.hpp>     // for Sync
#include <cloe/trigger.hpp>  // for Trigger, Event, EventFactory, ...

namespace engine {
namespace events {

class NextCallback;

class TimeEvent : public cloe::Event {
 public:
  TimeEvent(const std::string& name, cloe::Duration t) : Event(name), time_(t) {}
  cloe::Duration time() const { return time_; }
  cloe::EventPtr clone() const override { return std::make_unique<TimeEvent>(name(), time_); }
  void to_json(cloe::Json& j) const override {
    j = cloe::Json{
        {"time", cloe::Seconds{time_}.count()},
    };
  }

 private:
  friend NextCallback;

 private:
  cloe::Duration time_;
};

class TimeFactory : public cloe::EventFactory {
 public:
  TimeFactory() : cloe::EventFactory("time", "at simulation time") {}

  cloe::TriggerSchema schema() const override {
    using namespace cloe::schema;  // NOLINT(build/namespaces)
    static const char* desc = "absolute number of seconds in simulation time";
    return cloe::TriggerSchema{
        this->name(),
        this->description(),
        cloe::InlineSchema(desc, cloe::Json::value_t::number_float),
        cloe::Schema{
            {"time", Number<double>(nullptr, desc).require()},
        },
    };
  }

  cloe::EventPtr make(const cloe::Conf& c) const override {
    auto secs = cloe::Seconds{c.get<double>("time")};
    return std::make_unique<TimeEvent>(name(), std::chrono::duration_cast<cloe::Duration>(secs));
  }

  cloe::EventPtr make(const std::string& s) const override {
    return make(cloe::Conf{cloe::Json{
        {"time", std::stod(s)},
    }});
  }
};

struct TimeTrigger {
  TimeTrigger(cloe::Duration t, cloe::TriggerPtr&& tp) : time(t), trigger(std::move(tp)) {}

  friend void to_json(cloe::Json& j, const TimeTrigger& t) { j = t.trigger; }

 public:
  cloe::Duration time;
  cloe::TriggerPtr trigger;
};

using TimeEmplaceHook = std::function<void(const cloe::Trigger&, cloe::Duration)>;

struct TimeTriggerCompare : public std::binary_function<std::shared_ptr<TimeTrigger>,
                                                        std::shared_ptr<TimeTrigger>,
                                                        bool> {
  bool operator()(const std::shared_ptr<TimeTrigger>& x,
                  const std::shared_ptr<TimeTrigger>& y) const {
    return x->time > y->time;
  }
};

class TimeCallback : public cloe::Callback {
 public:
  TimeCallback(cloe::Logger log, TimeEmplaceHook h) : log_(log), hook_(h) {}

  void emplace(cloe::TriggerPtr&& t, const cloe::Sync& sync) override {
    auto now = sync.time();
    auto when = dynamic_cast<const TimeEvent&>(t->event()).time();
    hook_(*t, when);
    if (when < now) {
      log_->error("Inserting timed trigger for the past!");
      log_->error("> trigger time = {} s", std::chrono::duration_cast<cloe::Seconds>(when).count());
      log_->error("> current time = {} s", std::chrono::duration_cast<cloe::Seconds>(now).count());
    }
    if (t->is_sticky()) {
      log_->error("Inserting timed trigger that is sticky discards stickiness!");
    }
    storage_.emplace(std::make_shared<TimeTrigger>(when, std::move(t)));
  }

  void to_json(cloe::Json& j) const override {
    // Make a copy of the storage, and then empty it.
    decltype(storage_) st_copy{storage_};
    while (!st_copy.empty()) {
      j.push_back(st_copy.top());
      st_copy.pop();
    }
  }

  void trigger(const cloe::Sync& sync) {
    auto now = sync.time();
    while (!storage_.empty() && storage_.top()->time <= now) {
      auto tt = storage_.top();
      storage_.pop();
      this->execute(std::move(tt->trigger), sync);
    }
  }

 private:
  cloe::Logger log_;
  TimeEmplaceHook hook_;

  // This is ridiculous: We need to wrap the unique_ptr of Trigger in
  // a shared_ptr because you can only get objects by copy out of
  // a priority_queue.
  std::priority_queue<std::shared_ptr<TimeTrigger>,
                      std::vector<std::shared_ptr<TimeTrigger>>,
                      TimeTriggerCompare>
      storage_;
};

class NextFactory : public cloe::EventFactory {
 public:
  NextFactory() : cloe::EventFactory("next", "next step in simulation") {}

  cloe::TriggerSchema schema() const override {
    using namespace cloe::schema;  // NOLINT(build/namespaces)
    static const char* desc = "optional number of seconds from current simulation time";
    return cloe::TriggerSchema{
        name(),
        description(),
        cloe::InlineSchema(desc, cloe::Json::value_t::number_float, false),
        cloe::Schema{
            {"time", Number<double>(nullptr, desc)},
        },
    };
  }

  std::unique_ptr<cloe::Event> make(const cloe::Conf& c) const override {
    auto next_time = cloe::Duration{0};
    if (c.has("time")) {
      auto secs = cloe::Seconds{c.get<double>("time")};
      next_time += std::chrono::duration_cast<cloe::Duration>(secs);
    }
    return std::make_unique<TimeEvent>(name(), next_time);
  }

  std::unique_ptr<cloe::Event> make(const std::string& s) const override {
    if (!s.empty()) {
      return make(cloe::Conf{cloe::Json{
          {"time", std::stod(s)},
      }});
    }
    return make(cloe::Conf{});
  }
};

class NextCallback : public cloe::AliasCallback {
 public:
  using cloe::AliasCallback::AliasCallback;
  void emplace(cloe::TriggerPtr&& t, const cloe::Sync& s) override {
    auto& time_event = const_cast<TimeEvent&>(dynamic_cast<const TimeEvent&>(t->event()));
    time_event.set_name("time");
    time_event.time_ += s.time();
    cloe::AliasCallback::emplace(std::move(t), s);
  }
};

}  // namespace events
}  // namespace engine
