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
 * \file cloe/conf/action.hpp
 * \see  cloe/trigger.hpp
 */

#pragma once

#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <cloe/trigger.hpp>  // for Action, ActionFactory, TriggerSchema, InlineSchema, Confable

namespace cloe {
namespace actions {

class Configure : public Action {
 public:
  Configure(const std::string& name, Confable* ptr, const Conf& c)
      : Action(name), ptr_(ptr), conf_(c) {
    conf_.erase("name");
  }
  ActionPtr clone() const override { return std::make_unique<Configure>(name(), ptr_, conf_); }
  CallbackResult operator()(const Sync&, TriggerRegistrar&) override {
    ptr_->from_conf(conf_);
    return CallbackResult::Ok;
  }

 protected:
  void to_json(Json& j) const override { j = *conf_; }

 private:
  Confable* ptr_;
  Conf conf_;
};

class ConfigureFactory : public ActionFactory {
 public:
  using ActionType = Configure;
  ConfigureFactory(Confable* ptr, const std::string& name, const std::string& desc)
      : ActionFactory(name, desc), ptr_(ptr) {}

  TriggerSchema schema() const override {
    return TriggerSchema{
        name(),
        description(),
        InlineSchema(false),
        ptr_->schema(),
    };
  }

  ActionPtr make(const Conf& c) const override {
    return std::make_unique<Configure>(name(), ptr_, c);
  }

 private:
  Confable* ptr_;
};

}  // namespace actions
}  // namespace cloe
