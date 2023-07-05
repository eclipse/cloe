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
 * \file cloe/trigger/set_action.hpp
 *
 * This file defines actions that set a given variable.
 */

#pragma once

#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <cloe/trigger.hpp>                // for Action, ActionFactory
#include <cloe/trigger/helper_macros.hpp>  // for _X_FACTORY, _X_CALLBACK

namespace cloe {
namespace actions {

template <typename T>
T from_string(const std::string& s);

template <>
double from_string<double>(const std::string& s) {
  return std::stod(s);
}

template <>
int from_string<int>(const std::string& s) {
  return std::stoi(s);
}

template <>
bool from_string<bool>(const std::string& s) {
  if (s == "true") {
    return true;
  } else if (s == "false") {
    return false;
  } else {
    throw std::out_of_range("cannot parse into boolean: " + s);
  }
}

// Set configuration attributes via triggers.
template <typename T>
class SetVariableAction : public Action {
 public:
  explicit SetVariableAction(const std::string& action_name, const std::string& data_name,
                             T* data_ptr, T value)
      : Action(action_name), data_name_(data_name), data_ptr_(data_ptr), value_(value) {}
  ActionPtr clone() const override {
    return std::make_unique<SetVariableAction>(name(), data_name_, data_ptr_, value_);
  }
  CallbackResult operator()(const Sync&, TriggerRegistrar&) override {
    *data_ptr_ = value_;
    return CallbackResult::Ok;
  }
  bool is_significant() const override { return false; }
  void to_json(Json& j) const override {
    j = Json{
        {data_name_, value_},
    };
  }

 private:
  std::string data_name_;
  T* data_ptr_;
  T value_;
};

template <typename T>
class SetVariableActionFactory : public ActionFactory {
 public:
  using ActionType = SetVariableAction<T>;
  explicit SetVariableActionFactory(const std::string& action_name, const std::string& action_desc,
                                    const std::string& data_name, T* data_ptr)
      : ActionFactory(action_name, action_desc), data_name_(data_name), data_ptr_(data_ptr) {}
  ActionPtr make(const Conf& c) const override {
    auto value = c.get<T>(data_name_);
    return std::make_unique<SetVariableAction<T>>(name(), data_name_, data_ptr_, value);
  }
  ActionPtr make(const std::string& s) const override {
    auto value = from_string<T>(s);
    return make(Conf{Json{
        {data_name_, value},
    }});
  }

 private:
  std::string data_name_;
  T* data_ptr_;
};

}  // namespace actions
}  // namespace cloe

/**
 * Macro DEFINE_SET_STATE_ACTION defines an action that has only a single state
 * and no configuration.
 *
 * \param xName          type identifier for event
 * \param xname          string identifier of event (lowercase)
 * \param xdescription   string description of event
 * \param xState         type identifier for member state pointer
 * \param xOperatorBlock code block that makes use of ptr_
 *
 * Example
 * -------
 *
 * Given the type `Simulation` and the action "abort the simulation", we
 * can create it in code like so, note that the semicolon at the end is
 * required.
 *
 * ```
 * DEFINE_SET_STATE_ACTION(Abort, "abort", "abort simulation", Simulation, {
 *   ptr_->abort();
 * });
 * ```
 *
 * This will define the following classes for us:
 *
 * - `Abort`
 * - `AbortFactory`
 *
 * This action can be registered with the `register_action` helper function.
 */
#define DEFINE_SET_STATE_ACTION(xName, xname, xdescription, xState, xOperatorBlock)              \
  class xName : public ::cloe::Action {                                                          \
   public:                                                                                       \
    xName(const std::string& name, xState* ptr) : ::cloe::Action(name), ptr_(ptr) {}             \
    ::cloe::ActionPtr clone() const override { return std::make_unique<xName>(name(), ptr_); }   \
    ::cloe::CallbackResult operator()(const ::cloe::Sync&, ::cloe::TriggerRegistrar&) override { \
      xOperatorBlock;                                                                            \
      return ::cloe::CallbackResult::Ok;                                                         \
    }                                                                                            \
    void to_json(::cloe::Json&) const override {}                                                \
                                                                                                 \
   private:                                                                                      \
    xState* ptr_;                                                                                \
  };                                                                                             \
                                                                                                 \
  class _X_FACTORY(xName) : public ::cloe::ActionFactory {                                       \
   public:                                                                                       \
    using ActionType = xName;                                                                    \
                                                                                                 \
    _X_FACTORY(xName)(xState * ptr) : ::cloe::ActionFactory(xname, xdescription), ptr_(ptr) {}   \
                                                                                                 \
    ::cloe::ActionPtr make(const ::cloe::Conf&) const override {                                 \
      return std::make_unique<xName>(name(), ptr_);                                              \
    }                                                                                            \
                                                                                                 \
    ::cloe::ActionPtr make(const std::string&) const override {                                  \
      return std::make_unique<xName>(name(), ptr_);                                              \
    }                                                                                            \
                                                                                                 \
   private:                                                                                      \
    xState* ptr_;                                                                                \
  };

/**
 * Macro DEFINE_SET_DATA_ACTION defines an action that sets an attribute from
 * the configuration.
 *
 * \param xName           type identifier for action
 * \param xActionName     string identifier of action (lowercase)
 * \param xActionDesc     string description of action
 * \param xDataType       type identifier for member data pointer
 * \param xAttributeName  string identifier of the attribute (defines configuration)
 * \param xAttributeType  type identifier of the attribute
 * \param xOperatorBlock  code block that makes use of ptr_ and value_
 *
 * Example
 * -------
 *
 * Given the type `SimulationSync` and the action "modify the simulation speed",
 * we can create it in code like so:
 *
 * ```
 * DEFINE_SET_DATA_ACTION(RealtimeFactor, "realtime_factor", "modify simulation speed",
 *                        SimulationSync, "factor", double,
 *                        {
 *                         logger()->info("Setting target simulation speed: {}", value_);
 *                         ptr_->set_realtime_factor(value_);
 *                        })
 * ```
 *
 * This will define the following classes for us:
 *
 * - `RealtimeFactor`
 * - `RealtimeFactorFactory`
 *
 * This action can be registered with the `register_action` helper function.
 * Refer to doc/reference/actions.rst for the configuration.
 */
#define DEFINE_SET_DATA_ACTION(xName, xActionName, xActionDesc, xDataType, xAttributeName,       \
                               xAttributeType, xOperatorBlock)                                   \
  class xName : public ::cloe::Action {                                                          \
   public:                                                                                       \
    xName(const std::string& action_name, xDataType* ptr, const std::string& attribute_name,     \
          const xAttributeType attribute_value)                                                  \
        : ::cloe::Action(action_name)                                                            \
        , ptr_(ptr)                                                                              \
        , name_(attribute_name)                                                                  \
        , value_(attribute_value) {}                                                             \
    ::cloe::ActionPtr clone() const override {                                                   \
      return std::make_unique<xName>(name(), ptr_, name_, value_);                               \
    }                                                                                            \
    ::cloe::CallbackResult operator()(const ::cloe::Sync&, ::cloe::TriggerRegistrar&) override { \
      xOperatorBlock;                                                                            \
      return ::cloe::CallbackResult::Ok;                                                         \
    }                                                                                            \
    bool is_significant() const override { return false; }                                       \
    void to_json(::cloe::Json& j) const override {                                               \
      j = ::fable::Json{                                                                         \
          {name_, value_},                                                                       \
      };                                                                                         \
    }                                                                                            \
                                                                                                 \
   private:                                                                                      \
    xDataType* ptr_;                                                                             \
    std::string name_;                                                                           \
    xAttributeType value_;                                                                       \
  };                                                                                             \
                                                                                                 \
  class _X_FACTORY(xName) : public ::cloe::ActionFactory {                                       \
   public:                                                                                       \
    using ActionType = xName;                                                                    \
    _X_FACTORY(xName)                                                                            \
    (xDataType * ptr) : ::cloe::ActionFactory(xActionName, xActionDesc), ptr_(ptr) {}            \
                                                                                                 \
    ::cloe::ActionPtr make(const ::cloe::Conf& c) const override {                               \
      auto value = c.get<xAttributeType>(xAttributeName);                                        \
      return std::make_unique<xName>(name(), ptr_, xAttributeName, value);                       \
    }                                                                                            \
                                                                                                 \
    ::cloe::ActionPtr make(const std::string& s) const override {                                \
      auto value = ::cloe::actions::from_string<xAttributeType>(s);                              \
      return make(::fable::Conf{::fable::Json{                                                   \
          {xAttributeName, value},                                                               \
      }});                                                                                       \
    }                                                                                            \
                                                                                                 \
   private:                                                                                      \
    xDataType* ptr_;                                                                             \
  };
