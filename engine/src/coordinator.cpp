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

#include <functional>          // for bind
#include <memory>              // for unique_ptr<>, shared_ptr<>
#include <mutex>               // for unique_lock<>
#include <string>              // for string
#include <utility>             // for move
#include <vector>              // for vector<>

#include <cloe/core.hpp>       // for Json, Duration, Logger
#include <cloe/handler.hpp>    // for HandlerType, Request
#include <cloe/registrar.hpp>  // for Registrar
#include <cloe/sync.hpp>       // for Sync
#include <cloe/trigger.hpp>    // for Trigger
using namespace cloe;          // NOLINT(build/namespaces)

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

Coordinator::Coordinator(sol::state_view lua, cloe::DataBroker* db)
    : lua_(lua), executer_registrar_(trigger_registrar(Source::TRIGGER)), db_(db) {
  lua_["cloe"]["plugins"] = lua_.create_table();
}

class TriggerRegistrar : public cloe::TriggerRegistrar {
 public:
  TriggerRegistrar(Coordinator& m, Source s) : cloe::TriggerRegistrar(s), manager_(m) {}

  ActionPtr make_action(const Conf& c) const override { return manager_.make_action(c); }
  EventPtr make_event(const Conf& c) const override { return manager_.make_event(c); }
  TriggerPtr make_trigger(const Conf& c) const override {
    return manager_.make_trigger(source_, c);
  }

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
      for (const auto& a : actions_) {
        j[a.first] = a.second->json_schema();
      }
      r.write(j);
    }
  );
  r.register_api_handler("/triggers/events", HandlerType::STATIC,
    [this](const Request&, Response& r) {
      Json j;
      for (const auto& a : events_) {
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
  if (actions_.count(key) != 0) {
    throw Error("duplicate action name not allowed");
  }
  logger()->debug("Register action: {}", key);
  af->set_name(key);
  actions_[key] = std::move(af);
}

void Coordinator::register_event(const std::string& key, EventFactoryPtr&& ef,
                                 std::shared_ptr<Callback> storage) {
  if (events_.count(key) != 0) {
    throw Error("duplicate event name not allowed");
  }
  logger()->debug("Register event: {}", key);
  ef->set_name(key);
  events_[key] = std::move(ef);
  storage_[key] = storage;
  storage->set_executer(
      std::bind(&Coordinator::execute_trigger, this, std::placeholders::_1, std::placeholders::_2));
}

sol::table Coordinator::register_lua_table(const std::string& field) {
  auto tbl = lua_.create_table();
  lua_["cloe"]["plugins"][field] = tbl;
  return tbl;
}

cloe::CallbackResult Coordinator::execute_trigger(TriggerPtr&& t, const Sync& sync) {
  logger()->debug("Execute trigger {}", inline_json(*t));
  auto result = (t->action())(sync, *executer_registrar_);
  if (!t->is_conceal()) {
    history_.emplace_back(HistoryTrigger{sync.time(), std::move(t)});
  }
  return result;
}

Duration Coordinator::process(const Sync& sync) {
  // The only thing we need to do here is distribute the triggers from the
  // input queue into their respective storage locations. We are responsible
  // for thread safety here!
  auto now = sync.time();
  std::unique_lock guard(input_mutex_);
  while (!input_queue_.empty()) {
    auto tp = std::move(input_queue_.front());
    input_queue_.pop_front();
    tp->set_since(now);

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

  return now;
}

namespace {
template <typename T, typename M>
std::unique_ptr<T> make_some(const Conf& c, const M& m) {
  bool alternate = (*c).type() == Json::value_t::string;

  auto get_factory = [&](const std::string& key) -> const auto& {
    if (m.count(key) == 0) {
      if (std::is_same<T, Action>::value) {
        throw TriggerUnknownAction(key, c);
      }
      if (std::is_same<T, Event>::value) {
        throw TriggerUnknownEvent(key, c);
      }
    } else {
      return m.at(key);
    }
  };

  if (alternate) {
    // This section tries to create a new action/event by using the alternate
    // string description. Not every event/action supports this though!
    auto input = c.get<std::string>();

    std::string name;
    std::string argument;
    auto found = input.find("=");
    if (found == std::string::npos) {
      name = input;
    } else {
      name = input.substr(0, found);
      argument = input.substr(found + 1);
    }

    const auto& factory = get_factory(name);
    try {
      return factory->make(argument);
    } catch (TriggerError& e) {
      c.throw_error(e.what());
    }
  } else {
    auto name = c.get<std::string>("name");
    const auto& factory = get_factory(name);
    return factory->make(c);
  }
}

}  // anonymous namespace

ActionPtr Coordinator::make_action(const Conf& c) const { return make_some<Action>(c, actions_); }

EventPtr Coordinator::make_event(const Conf& c) const { return make_some<Event>(c, events_); }

TriggerPtr Coordinator::make_trigger(Source s, const Conf& c) const {
  EventPtr ep;
  ActionPtr ap;
  bool opt = c.get_or("optional", false);
  try {
    ep = make_event(c.at("event"));
    ap = make_action(c.at("action"));
  } catch (TriggerError& e) {
    if (opt) {
      logger()->warn("Ignoring optional trigger ({}): {}", e.what(), c->dump());
      return nullptr;
    } else {
      throw;
    }
  }
  auto label = c.get_or<std::string>("label", "");
  auto t = std::make_unique<Trigger>(label, s, std::move(ep), std::move(ap));
  t->set_sticky(c.get_or("sticky", false));
  t->set_conceal(c.get_or("conceal", false));
  return t;
}

void Coordinator::queue_trigger(TriggerPtr&& t) {
  if (t == nullptr) {
    // This only really happens if a trigger is an optional trigger.
    return;
  }
  std::unique_lock guard(input_mutex_);
  input_queue_.emplace_back(std::move(t));
}

}  // namespace engine
