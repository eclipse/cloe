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
 * \file cloe/trigger/example_actions.hpp
 * \see  cloe/trigger/example_actions.cpp
 *
 * This file contains several useful actions. These are documented in the
 * Available Actions section in the Reference chapter.
 */

#pragma once

#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <cloe/trigger.hpp>  // for Action, ActionFactory

namespace cloe {
namespace actions {

// ----------------------------------------------------------------------- Log
class Log : public Action {
 public:
  Log(const std::string& name, LogLevel level, const std::string& msg)
      : Action(name), level_(level), msg_(msg) {}
  ActionPtr clone() const override { return std::make_unique<Log>(name(), level_, msg_); }
  CallbackResult operator()(const Sync&, TriggerRegistrar&) override {
    logger()->log(level_, msg_.c_str());
    return CallbackResult::Ok;
  }
  bool is_significant() const override { return false; }

 protected:
  void to_json(Json& j) const override {
    j = Json{
        {"level", logger::to_string(level_)},
        {"msg", msg_},
    };
  }

 private:
  LogLevel level_;
  std::string msg_;
};

class LogFactory : public ActionFactory {
 public:
  using ActionType = Log;
  LogFactory() : ActionFactory("log", "log a message with a severity") {}
  TriggerSchema schema() const override;
  ActionPtr make(const Conf& c) const override;
  ActionPtr make(const std::string& s) const override;
};

// -------------------------------------------------------------------- Bundle
class Bundle : public Action {
 public:
  Bundle(const std::string& name, std::vector<ActionPtr>&& actions);
  ActionPtr clone() const override;
  CallbackResult operator()(const Sync& s, TriggerRegistrar& r) override;
  bool is_significant() const override;

 protected:
  void to_json(Json& j) const override {
    j = Json{
        {"actions", repr_},
    };
  }

 private:
  std::vector<ActionPtr> actions_;
  Json repr_;
};

class BundleFactory : public ActionFactory {
 public:
  using ActionType = Bundle;
  explicit BundleFactory(std::shared_ptr<TriggerRegistrar> r)
      : ActionFactory("bundle", "run a set of actions"), registrar_(r) {}
  TriggerSchema schema() const override;
  ActionPtr make(const Conf& c) const override;

 private:
  std::shared_ptr<TriggerRegistrar> registrar_;
};

// -------------------------------------------------------------------- Insert
class Insert : public Action {
 public:
  Insert(const std::string& name, const Conf& triggers) : Action(name), triggers_(triggers) {}
  ActionPtr clone() const override { return std::make_unique<Insert>(name(), triggers_); }
  CallbackResult operator()(const Sync& s, TriggerRegistrar& r) override;

 protected:
  void to_json(Json& j) const override;

 private:
  Conf triggers_;
};

class InsertFactory : public ActionFactory {
 public:
  using ActionType = Insert;
  explicit InsertFactory(std::shared_ptr<TriggerRegistrar> r)
      : ActionFactory("insert", "insert a new trigger"), registrar_(r) {}
  TriggerSchema schema() const override;
  ActionPtr make(const Conf& c) const override;

 private:
  std::shared_ptr<TriggerRegistrar> registrar_;
};

// --------------------------------------------------------------- PushRelease
class PushRelease : public Action {
 public:
  PushRelease(const std::string& name, Duration dur, ActionPtr&& push, ActionPtr&& release,
              const Json& repr)
      : Action(name)
      , duration_(dur)
      , push_(push.release())
      , release_(release.release())
      , repr_(repr) {}
  ActionPtr clone() const override {
    return std::make_unique<PushRelease>(name(), duration_, push_->clone(), release_->clone(),
                                         repr_);
  }
  CallbackResult operator()(const Sync&, TriggerRegistrar&) override;

 protected:
  void to_json(Json& j) const override { j = repr_; }

 private:
  Duration duration_;
  ActionPtr push_;
  ActionPtr release_;
  Json repr_;
};

class PushReleaseFactory : public ActionFactory {
 public:
  using ActionType = PushRelease;
  explicit PushReleaseFactory(std::shared_ptr<TriggerRegistrar> r)
      : ActionFactory("push_release", "push and release one or more buttons"), registrar_(r) {}
  TriggerSchema schema() const override;
  ActionPtr make(const Conf& c) const override;

 private:
  std::shared_ptr<TriggerRegistrar> registrar_;
};

}  // namespace actions
}  // namespace cloe
