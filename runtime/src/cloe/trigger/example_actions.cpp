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
 * \file cloe/trigger/example_actions.cpp
 * \see  cloe/trigger/example_actions.hpp
 */

#include <cloe/trigger/example_actions.hpp>

#include <cstdlib>  // for system
#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <vector>   // for vector<>

#include <cloe/core.hpp>     // for Conf
#include <cloe/sync.hpp>     // for Sync
#include <cloe/trigger.hpp>  // for TriggerRegistrar

namespace cloe {
namespace actions {

// Log -----------------------------------------------------------------------
TriggerSchema LogFactory::schema() const {
  return TriggerSchema{
      this->name(),
      this->description(),
      InlineSchema("level and message to send", "[level:] msg", true),
      Schema{
          {"level", make_prototype<std::string>("logging level to use")},
          {"msg", make_prototype<std::string>("message to send").require()},
      },
  };
}

ActionPtr LogFactory::make(const Conf& c) const {
  auto level = logger::into_level(c.get_or<std::string>("level", "info"));
  return std::make_unique<Log>(name(), level, c.get<std::string>("msg"));
}

ActionPtr LogFactory::make(const std::string& s) const {
  auto level = spdlog::level::info;
  auto pos = s.find(":");
  std::string msg;
  if (pos != std::string::npos) {
    try {
      level = logger::into_level(s.substr(0, pos));
      if (s[++pos] == ' ') {  // ignore ':'
        ++pos;                // ignore ' '
      }
      msg = s.substr(pos);
    } catch (...) {
      // TODO(ben): Replace ... with specific error class
      // There is a ':', but whatever is before it isn't a logging level.
      msg = s;
    }
  } else {
    msg = s;
  }

  auto c = Conf{Json{
      {"level", logger::to_string(level)},
      {"msg", msg},
  }};
  if (msg.size() == 0) {
    throw TriggerInvalid(c, "cannot log an empty message");
  }
  return make(c);
}

// Bundle --------------------------------------------------------------------
Bundle::Bundle(const std::string& name, std::vector<ActionPtr>&& actions)
    : Action(name), actions_(std::move(actions)) {
  // Save the current JSON representation, because after operator()
  // it won't be possible anymore.
  repr_ = actions_;
}

bool Bundle::is_significant() const {
  for (const auto& x : actions_) {
    if (x->is_significant()) {
      return true;
    }
  }
  return false;
}

ActionPtr Bundle::clone() const {
  std::vector<ActionPtr> actions;
  actions.reserve(actions_.size());
  for (const auto& a : actions_) {
    actions.emplace_back(a->clone());
  }
  return std::make_unique<Bundle>(name(), std::move(actions));
}

CallbackResult Bundle::operator()(const Sync& sync, TriggerRegistrar& r) {
  logger()->trace("Run action bundle");
  CallbackResult result;
  for (auto& a : actions_) {
    auto ar = (*a)(sync, r);
    if (ar == CallbackResult::Unpin) {
      result = ar;
    }
  }
  return result;
}

TriggerSchema BundleFactory::schema() const {
  return TriggerSchema{
      this->name(),
      this->description(),
      Schema{
          {"actions", make_prototype<std::vector<Conf>>("action definitions to execute").require()},
      },
  };
}

ActionPtr BundleFactory::make(const Conf& c) const {
  c.assert_has_type("actions", JsonType::array);
  std::vector<ActionPtr> actions;
  for (const auto& ac : c.at("actions").to_array()) {
    actions.emplace_back(registrar_->make_action(ac));
  }
  return std::make_unique<Bundle>(name(), std::move(actions));
}

// Insert --------------------------------------------------------------------
void Insert::to_json(Json& j) const {
  j = Json{
      {"triggers", *triggers_},
  };
}

CallbackResult Insert::operator()(const Sync&, TriggerRegistrar& r) {
  for (const auto& tc : triggers_.to_array()) {
    auto local = r.make_trigger(tc);
    r.insert_trigger(std::move(local));
  }
  return CallbackResult::Ok;
}

TriggerSchema InsertFactory::schema() const {
  return TriggerSchema{
      this->name(),
      this->description(),
      Schema{
          {"triggers",
           make_prototype<std::vector<Conf>>("trigger definitions to insert").require()},
      },
  };
}

ActionPtr InsertFactory::make(const Conf& c) const {
  c.assert_has_type("triggers", JsonType::array);
  for (const auto& tc : c.at("triggers").to_array()) {
    // Make sure that we can make these actions later by making them now and
    // throwing them away.
    registrar_->make_trigger(tc);
  }
  return std::make_unique<Insert>(name(), c.at("triggers"));
}

// PushRelease ---------------------------------------------------------------
CallbackResult PushRelease::operator()(const Sync&, TriggerRegistrar& r) {
  // clang-format off
  r.insert_trigger(
    "push down button(s)",
    r.make_event(Conf{Json{
        {"name", "next"},
    }}),
    std::move(push_)
  );
  r.insert_trigger(
    "release button(s)",
    r.make_event(Conf{Json{
        {"name", "next"},
        {"time", std::chrono::duration_cast<Seconds>(duration_).count()},
    }}),
    std::move(release_)
  );
  return CallbackResult::Ok;
  // clang-format on
}

TriggerSchema PushReleaseFactory::schema() const {
  // clang-format off
  return TriggerSchema{
      this->name(),
      this->description(),
      Schema{
          {"action", make_prototype<std::string>("action name to use, e.g. basic/hmi").require()},
          {"duration", make_prototype<double>("duration in seconds to push button, e.g. 0.5").require()},
          {"buttons", make_schema(static_cast<std::vector<std::string>*>(nullptr), "list of buttons to activate").require()},
      },
  };
  // clang-format on
}

ActionPtr PushReleaseFactory::make(const Conf& c) const {
  auto action = c.get<std::string>("action");
  auto dur = std::chrono::duration_cast<Duration>(Seconds{c.get<double>("duration")});
  if (dur <= Duration(0)) {
    throw TriggerInvalid(c, "require a duration greater than zero");
  }
  auto buttons = c.get<std::vector<std::string>>("buttons");
  if (buttons.empty()) {
    throw TriggerInvalid(c, "refuse to create action push_release with no buttons");
  }

  auto create = [&](bool val) -> ActionPtr {
    auto j = Json{
        {"name", action},
    };
    for (const auto& b : buttons) {
      j[b] = val;
    }
    return registrar_->make_action(Conf{j});
  };

  auto repr = Json{
      {"action", action},
      {"duration", c.get<double>("duration")},
      {"buttons", buttons},
  };

  return std::make_unique<PushRelease>(name(), dur, create(true), create(false), repr);
}

}  // namespace actions
}  // namespace cloe
