/*
 * Copyright 2024 Robert Bosch GmbH
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

#pragma once

#include <memory>
#include <thread>

#include <cloe/trigger.hpp>
#include <fable/utility/chrono.hpp>

namespace cloe::actions {

class WallClockDelay : public Action {
 public:
  WallClockDelay(const std::string& name, cloe::Duration sleep_for,
                 std::function<bool(const Sync&)> func)
      : Action(name), sleep_for_(sleep_for), func_(std::move(func)) {}

  [[nodiscard]] ActionPtr clone() const override {
    return std::make_unique<WallClockDelay>(name(), sleep_for_, func_);
  }

  CallbackResult operator()(const Sync& sync, TriggerRegistrar& /*reg*/) override {
    if (func_ && !func_(sync)) {
      return CallbackResult::Ok;
    }

    std::this_thread::sleep_for(sleep_for_);
    return CallbackResult::Ok;
  }

  [[nodiscard]] bool is_significant() const override { return true; }

 protected:
  void to_json(Json& j) const override {
    j = Json{
        {"sleep_for", fable::to_string(sleep_for_)},
    };
  }

 private:
  cloe::Duration sleep_for_;
  std::function<bool(const Sync&)> func_;
};

class WallClockDelayFactory : public ActionFactory {
 public:
  using ActionType = WallClockDelay;

  WallClockDelayFactory(std::string name, std::string desc, std::function<bool(const Sync&)> func)
      : ActionFactory(std::move(name), std::move(desc)), func_(std::move(func)) {}

  [[nodiscard]] TriggerSchema schema() const override {
    return TriggerSchema{
        this->name(), this->description(), InlineSchema("time to delay for", "duration", true),
        fable::Schema{
            {"sleep_for", make_prototype<std::string>("time to delay for").require()},
        }};
  }

  [[nodiscard]] ActionPtr make(const Conf& c) const override {
    return std::make_unique<WallClockDelay>(
        name(), fable::parse_duration<cloe::Duration>(c.get<std::string>("sleep_for")), func_);
  }

  [[nodiscard]] ActionPtr make(const std::string& s) const override {
    return make(Conf(Json{
        {"sleep_for", s},
    }));
  }

 private:
  std::function<bool(const Sync&)> func_;
};

}  // namespace cloe::actions
