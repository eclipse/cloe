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
 * \file cloe/trigger.cpp
 * \see  cloe/trigger.hpp
 */

#include <cloe/trigger.hpp>

#include <string>   // for string
#include <utility>  // for move

#include <cloe/core.hpp>  // for Json

namespace cloe {

InlineSchema::InlineSchema(std::string desc, JsonType type, bool required)
    : type_(type)
    , required_(required)
    , usage_("<" + fable::to_string(type) + ">")
    , desc_(std::move(desc)) {
  assert(type != JsonType::null && "Use InlineSchema(std::string&&) when type is null");
  assert(type != JsonType::array && type != JsonType::object &&
         "Use InlineSchema(std::string&&, std::string&&, bool) when type is not primitive");
}

std::string InlineSchema::usage(const std::string& name) const {
  if (!is_enabled()) {
    return "";
  }
  if (type_ == JsonType::null) {
    return name;
  }
  if (is_required()) {
    return fmt::format("{}={}", name, usage_);
  } else {
    return fmt::format("{}[={}]", name, usage_);
  }
}

Json TriggerSchema::json_schema() const {
  Json j = schema_.json_schema_qualified("trigger/" + name());
  j["title"] = name();
  j["inline"] = inline_.usage(name_);
  j["type"] = "object";
  j["properties"]["name"] = Json{
      {"const", name()},
  };
  j["required"].push_back("name");
  return j;
}

Trigger::Trigger(const std::string& label, Source s, EventPtr&& e, ActionPtr&& a)
    : label_(label), source_(s), event_(std::move(e)), action_(std::move(a)) {}

TriggerPtr Trigger::clone() const {
  return std::make_unique<Trigger>(label_, Source::INSTANCE, event_->clone(), action_->clone());
}

bool Trigger::is_significant() const { return action_->is_significant(); }

void Trigger::set_conceal(bool value) {
  if (!value) {
    conceal_ = false;
  } else {
    if (is_significant()) {
      throw std::runtime_error("cannot conceal significant trigger");
    }
    conceal_ = true;
  }
}

void Trigger::set_sticky(bool value) { sticky_ = value; }

void to_json(Json& j, const Trigger& t) {
  j = Json{
      {"label", t.label()}, {"source", t.source()}, {"since", t.since()},
      {"event", t.event()}, {"action", t.action()}, {"sticky", t.is_sticky()},
  };
}

void to_json(Json& j, const Event& e) {
  e.to_json(j);
  ::cloe::to_json(j, dynamic_cast<const Entity&>(e));
}

void to_json(Json& j, const Action& e) {
  e.to_json(j);
  ::cloe::to_json(j, dynamic_cast<const Entity&>(e));
}

void TriggerRegistrar::insert_trigger(const std::string& label, EventPtr&& e, ActionPtr&& a) {
  insert_trigger(std::make_unique<Trigger>(label, source_, std::move(e), std::move(a)));
}

void Callback::execute(TriggerPtr&& t, const Sync& sync) {
  assert(executer_);
  executer_(std::move(t), sync);
}

}  // namespace cloe
