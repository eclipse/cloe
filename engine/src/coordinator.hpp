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

#include <list>                // for list<>
#include <map>                 // for map<>
#include <memory>              // for unique_ptr<>, shared_ptr<>
#include <mutex>               // for mutex
#include <queue>               // for queue<>
#include <string>              // for string
#include <vector>              // for vector<>

#include <cloe/cloe_fwd.hpp>   // for DataBroker
#include <cloe/trigger.hpp>    // for Trigger, Action, Event, ...

#include "simulation_driver.hpp"
#include "trigger_factory.hpp"

namespace engine {

// Forward declarations:
class TriggerRegistrar;  // from coordinator.cpp

struct HistoryTrigger {
  HistoryTrigger(cloe::Duration d, cloe::TriggerPtr&& t) : when(d), what(std::move(t)) {}

  friend void to_json(cloe::Json& j, const HistoryTrigger& t);

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
  Coordinator(SimulationDriver* simulation_driver, cloe::DataBroker* db);

  const std::vector<HistoryTrigger>& history() const { return history_; }

  void register_action(const std::string& key, cloe::ActionFactoryPtr&& af);

  void register_event(const std::string& key, cloe::EventFactoryPtr&& ef,
                      std::shared_ptr<cloe::Callback> storage);

  sol::table register_lua_table(const std::string& field);

  cloe::DataBroker* data_broker() const { return db_; }

  std::shared_ptr<cloe::TriggerRegistrar> trigger_registrar(cloe::Source s);

  void enroll(cloe::Registrar& r);

  static cloe::Logger logger() { return cloe::logger::get("cloe"); }

  /**
   * Process any incoming triggers, clear the buffer, and trigger time-based
   * events.
   */
  cloe::Duration process(const cloe::Sync&);

  size_t process_pending_driver_triggers(const cloe::Sync& sync);
  size_t process_pending_web_triggers(const cloe::Sync& sync);

  void insert_trigger(const cloe::Sync& sync, cloe::TriggerPtr trigger);
  void execute_action(const cloe::Sync& sync, cloe::Action& action);

  TriggerFactory& trigger_factory();
  const TriggerFactory& trigger_factory() const;

  SimulationDriver& simulation_driver();

 protected:
  void queue_trigger(cloe::Source s, const cloe::Conf& c);
  void queue_trigger(cloe::TriggerPtr&& tp);
  void store_trigger(cloe::TriggerPtr&& tp, const cloe::Sync& sync);
  cloe::CallbackResult execute_trigger(cloe::TriggerPtr&& tp, const cloe::Sync& sync);

  // for access to protected methods
  friend TriggerRegistrar;

 private:
  // Options:
  bool allow_errors_ = false; // todo never written to, always false!

  // Factories:
  std::unique_ptr<TriggerFactory> trigger_factory_;
  SimulationDriver* simulation_driver_;
  cloe::DataBroker* db_;  // non-owning

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
