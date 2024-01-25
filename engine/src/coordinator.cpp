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
 * \file coordinator.cpp
 * \see  coordinator.hpp
 */

#include "coordinator.hpp"

#include <functional>  // for bind
#include <memory>      // for unique_ptr<>, shared_ptr<>
#include <mutex>       // for unique_lock<>
#include <string>      // for string
#include <utility>     // for move
#include <vector>      // for vector<>

#include <fable/utility/sol.hpp>
#include <sol/optional.hpp>

#include <cloe/core.hpp>       // for Json, Duration, Logger
#include <cloe/handler.hpp>    // for HandlerType, Request
#include <cloe/registrar.hpp>  // for Registrar
#include <cloe/sync.hpp>       // for Sync
#include <cloe/trigger.hpp>    // for Trigger
using namespace cloe;          // NOLINT(build/namespaces)

#include "lua_action.hpp"
#include "lua_api.hpp"

namespace engine {

template <typename T>
std::string inline_json(const T& x) {
  ::Json tmp;
  to_json(tmp, x);
  return tmp.dump();
}

void to_json(Json& j, const HistoryTrigger& t) {
  to_json(j, *t.what);
  j["at"] = t.when;
}

Coordinator::Coordinator(SimulationDriver* simulation_driver, cloe::DataBroker* db)
    : simulation_driver_(simulation_driver), db_(db), executer_registrar_(trigger_registrar(Source::TRIGGER)) {}

class TriggerRegistrar : public cloe::TriggerRegistrar {
 public:
  TriggerRegistrar(Coordinator& m, Source s) : cloe::TriggerRegistrar(s), manager_(m) {}

  ActionPtr make_action(const Conf& c) const override { return manager_.trigger_factory().make_action(c); }
  EventPtr make_event(const Conf& c) const override { return manager_.trigger_factory().make_event(c); }
  TriggerPtr make_trigger(const Conf& c) const override {
    return manager_.trigger_factory().make_trigger(source_, c);
  }

  // TODO: Should these queue_trigger becomes inserts? Because if they are coming from
  // C++ then they should be from a single thread.
  void insert_trigger(const Conf& c) override { manager_.queue_trigger(source_, c); }
  void insert_trigger(TriggerPtr&& t) override { manager_.queue_trigger(std::move(t)); }

 private:
  Coordinator& manager_;
};

std::shared_ptr<cloe::TriggerRegistrar> Coordinator::trigger_registrar(Source s) {
  return std::make_shared<TriggerRegistrar>(*this, s);
}

void Coordinator::enroll(Registrar& r) {
  // clang-format off
  r.register_api_handler("/triggers/actions", HandlerType::STATIC,
    [this](const Request&, Response& r) {
      Json j;
      for (const auto& a : trigger_factory_->actions()) {
        j[a.first] = a.second->json_schema();
      }
      r.write(j);
    }
  );
  r.register_api_handler("/triggers/events", HandlerType::STATIC,
    [this](const Request&, Response& r) {
      Json j;
      for (const auto& a : trigger_factory_->events()) {
        j[a.first] = a.second->json_schema();
      }
      r.write(j);
    }
  );
  r.register_api_handler("/triggers/history", HandlerType::BUFFERED,
    [this](const Request&, Response& r) {
      r.write(history_);
    }
  );
  r.register_api_handler("/triggers/queue", HandlerType::BUFFERED,
    [this](const Request&, Response& r) {
      Json j;
      for (const auto& callback : storage_) {
        if (dynamic_cast<const AliasCallback*>(callback.second.get())) {
          // Ignore aliases, because they never contain any elements.
          continue;
        }
        j[callback.first] = callback.second;
      }
      r.write(j);
    }
  );
  r.register_api_handler("/triggers/input", HandlerType::STATIC,
    [this](const Request& q, Response& r) {
      // We are responsible for thread-safety!
      switch (q.method()) {
      case RequestMethod::GET:
        {
          std::unique_lock guard(input_mutex_);
          r.write(input_queue_);
          break;
        }
      case RequestMethod::POST:
        {
          try {
            queue_trigger(Source::NETWORK, Conf{q.as_json()});
          } catch(std::exception& e) {
            logger()->error("Error inserting trigger: {}", e.what());
            logger()->error("> Trigger definition: {}", q.as_json().dump(2));
            r.bad_request(Json{
                {"error", e.what()},
            });
          }
          break;
        }
      default:
        {
          r.not_allowed(RequestMethod::POST, Json{
              {"error", "only GET or POST method allowed"},
          });
        }
      }
    }
  );
  // clang-format on
}

void Coordinator::register_action(const std::string& key, ActionFactoryPtr&& af) {
  trigger_factory_->register_action(key, std::move(af));
}

void Coordinator::register_event(const std::string& key, EventFactoryPtr&& ef,
                                 std::shared_ptr<Callback> storage) {
  trigger_factory_->register_event(key, std::move(ef));
  storage_[key] = storage;
  storage->set_executer(
      std::bind(&Coordinator::execute_trigger, this, std::placeholders::_1, std::placeholders::_2));
}

cloe::CallbackResult Coordinator::execute_trigger(TriggerPtr&& t, const Sync& sync) {
  logger()->debug("Execute trigger {}", inline_json(*t));
  auto result = (t->action())(sync, *executer_registrar_);
  if (!t->is_conceal()) {
    history_.emplace_back(sync.time(), std::move(t));
  }
  return result;
}

void Coordinator::insert_trigger(const cloe::Sync& sync, cloe::TriggerPtr trigger) {
  store_trigger(std::move(trigger), sync);
}

size_t Coordinator::process_pending_web_triggers(const Sync& sync) {
  // The only thing we need to do here is distribute the triggers from the
  // input queue into their respective storage locations. We are responsible
  // for thread safety here!
  size_t count = 0;
  std::unique_lock guard(input_mutex_);
  while (!input_queue_.empty()) {
    store_trigger(std::move(input_queue_.front()), sync);
    input_queue_.pop_front();
    count++;
  }
  return count;
}

void Coordinator::store_trigger(TriggerPtr&& tp, const Sync& sync) {
  tp->set_since(sync.time());

  // Decide where to put the trigger
  auto key = tp->event().name();
  if (storage_.count(key) == 0) {
    // This is a programming error, since we should not be able to come this
    // far at all.
    throw std::logic_error("cannot insert trigger with unregistered event");
  }
  try {
    logger()->debug("Insert trigger {}", inline_json(*tp));
    storage_[key]->emplace(std::move(tp), sync);
  } catch (TriggerError& e) {
    logger()->error("Error inserting trigger: {}", e.what());
    if (!allow_errors_) {
      throw;
    }
  }
}

Duration Coordinator::process(const Sync& sync) {
  auto now = sync.time();
  process_pending_web_triggers(sync);
  return sync.time();
}

void Coordinator::queue_trigger(TriggerPtr&& t) {
  if (t == nullptr) {
    // This only really happens if a trigger is an optional trigger.
    return;
  }
  std::unique_lock guard(input_mutex_);
  input_queue_.emplace_back(std::move(t));
}

void Coordinator::queue_trigger(cloe::Source s, const Conf& c) {
  queue_trigger(trigger_factory_->make_trigger(s, c));
}

size_t Coordinator::process_pending_driver_triggers(const Sync& sync) {
  auto triggers = simulation_driver_->yield_pending_triggers(*trigger_factory_);
  const auto count = triggers.size();
  for(auto &&trigger : triggers) {
    store_trigger(std::move(trigger), sync);
  }
  return count;
}

sol::table Coordinator::register_lua_table(const std::string& field) {
  auto tbl = lua_.create_table();
  luat_cloe_engine_plugins(lua_)[field] = tbl;
  return tbl;
}
TriggerFactory& Coordinator::trigger_factory() { return *trigger_factory_; }
const TriggerFactory& Coordinator::trigger_factory() const { return *trigger_factory_; }
SimulationDriver& Coordinator::simulation_driver() { return *simulation_driver_; }
void Coordinator::execute_action(const Sync& sync, cloe::Action& action) {
  action(sync, *executer_registrar_);
}

}  // namespace engine
