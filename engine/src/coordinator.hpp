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
 * \file coordinator.hpp
 * \see  coordinator.cpp
 */

#pragma once

#include <list>    // for list<>
#include <map>     // for map<>
#include <memory>  // for unique_ptr<>, shared_ptr<>
#include <mutex>   // for mutex
#include <queue>   // for queue<>
#include <string>  // for string
#include <vector>  // for vector<>

#include <sol/state_view.hpp>  // for state_view
#include <sol/table.hpp>       // for table

#include <cloe/trigger.hpp>  // for Trigger, Action, Event, ...

// Forward declaration:
namespace cloe {
class Registrar;
}

namespace engine {

// Forward declarations:
class TriggerRegistrar;  // from coordinator.cpp

/**
 * TriggerUnknownAction is thrown when an Action cannot be created because the
 * ActionFactory cannot be found.
 */
class TriggerUnknownAction : public cloe::TriggerInvalid {
 public:
  TriggerUnknownAction(const std::string& key, const cloe::Conf& c)
      : TriggerInvalid(c, "unknown action: " + key), key_(key) {}
  virtual ~TriggerUnknownAction() noexcept = default;

  /**
   * Return key that is unknown.
   */
  const char* key() const { return key_.c_str(); }

 private:
  std::string key_;
};

/**
 * TriggerUnknownEvent is thrown when an Event cannot be created because the
 * EventFactory cannot be found.
 */
class TriggerUnknownEvent : public cloe::TriggerInvalid {
 public:
  TriggerUnknownEvent(const std::string& key, const cloe::Conf& c)
      : TriggerInvalid(c, "unknown event: " + key), key_(key) {}
  virtual ~TriggerUnknownEvent() noexcept = default;

  /**
   * Return key that is unknown.
   */
  const char* key() const { return key_.c_str(); }

 private:
  std::string key_;
};

struct HistoryTrigger {
  HistoryTrigger(cloe::Duration d, cloe::TriggerPtr&& t) : when(d), what(std::move(t)) {}

  friend void to_json(cloe::Json& j, const HistoryTrigger& t);

 public:
  cloe::Duration when;
  cloe::TriggerPtr what;
};

/**
 * Coordinator manages the set of available triggers as well as the concrete
 * list of active trigger events.
 *
 * Before it can be configured, it is important that all simulators, components,
 * controllers, etc. have registered their triggers and actions.
 */
class Coordinator {
 public:
  Coordinator(sol::state_view lua);

  const std::vector<HistoryTrigger>& history() const { return history_; }

  void register_action(const std::string& key, cloe::ActionFactoryPtr&& af);

  void register_event(const std::string& key, cloe::EventFactoryPtr&& ef,
                      std::shared_ptr<cloe::Callback> storage);

  sol::table register_lua_table(const std::string& key);

  std::shared_ptr<cloe::TriggerRegistrar> trigger_registrar(cloe::Source s);

  void enroll(cloe::Registrar& r);

  cloe::Logger logger() const { return cloe::logger::get("cloe"); }

  /**
   * Process any incoming triggers, clear the buffer, and trigger time-based
   * events.
   */
  cloe::Duration process(const cloe::Sync&);

 protected:
  cloe::ActionPtr make_action(const cloe::Conf& c) const;
  cloe::EventPtr make_event(const cloe::Conf& c) const;
  cloe::TriggerPtr make_trigger(cloe::Source s, const cloe::Conf& c) const;
  void queue_trigger(cloe::Source s, const cloe::Conf& c) { queue_trigger(make_trigger(s, c)); }
  void queue_trigger(cloe::TriggerPtr&& t);
  void execute_trigger(cloe::TriggerPtr&& t, const cloe::Sync& s);

  // for access to protected methods
  friend TriggerRegistrar;

 private:
  // Options:
  bool allow_errors_ = false;

  // Factories:
  std::map<std::string, cloe::ActionFactoryPtr> actions_;
  std::map<std::string, cloe::EventFactoryPtr> events_;
  sol::state_view lua_;

  // Execution:
  std::shared_ptr<cloe::TriggerRegistrar> executer_registrar_;

  // Storage:
  std::map<std::string, std::shared_ptr<cloe::Callback>> storage_;

  // Input:
  std::list<cloe::TriggerPtr> input_queue_;
  mutable std::mutex input_mutex_;

  // History:
  std::vector<HistoryTrigger> history_;
};

}  // namespace engine
