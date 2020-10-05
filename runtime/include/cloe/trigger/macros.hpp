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
 * \file cloe/trigger/macros.hpp
 */

#pragma once
#ifndef CLOE_TRIGGER_MACROS_HPP_
#define CLOE_TRIGGER_MACROS_HPP_

#include <memory>  // for unique_ptr<>, make_unique<>
#include <string>  // for string

#include <cloe/core.hpp>     // for Conf, Json
#include <cloe/trigger.hpp>  // for Event, EventFactory, Action, ActionFactory

#define _X_FACTORY(xName) xName##Factory
#define _X_CALLBACK(xName) xName##Callback

/**
 * Macro DEFINE_NIL_EVENT defines an event that has no state and no
 * configuration.
 *
 * \param xName         identifier for event
 * \param xname         string identifier of event (lowercase)
 * \param xdescription  string description of event
 *
 * Example
 * -------
 *
 * Given the stateless event "start of simulation", we can create it in code
 * like so, note that the semicolon at the end is required.
 *
 * ```
 * DEFINE_NIL_EVENT(Start, "start", "start of simulation")
 * ```
 *
 * This will define the following classes for us:
 *
 * - `Event`
 * - `EventFactory`
 * - `EventCallback`
 *
 * This event can be registered with the `register_event` helper function.
 */
#define DEFINE_NIL_EVENT(xName, xname, xdescription)                                    \
  class xName : public ::cloe::Event {                                                  \
   public:                                                                              \
    explicit xName(const std::string& name) : ::cloe::Event(name) {}                    \
    ::cloe::EventPtr clone() const override { return std::make_unique<xName>(name()); } \
    void to_json(::cloe::Json&) const override {}                                       \
    bool operator()(const ::cloe::Sync&) const { return true; }                         \
  };                                                                                    \
                                                                                        \
  class _X_FACTORY(xName) : public ::cloe::EventFactory {                               \
   public:                                                                              \
    using EventType = xName;                                                            \
                                                                                        \
    _X_FACTORY(xName)() : ::cloe::EventFactory(xname, xdescription) {}                  \
                                                                                        \
    std::unique_ptr<::cloe::Event> make(const ::cloe::Conf&) const override {           \
      return std::make_unique<xName>(this->name());                                     \
    }                                                                                   \
                                                                                        \
    std::unique_ptr<::cloe::Event> make(const std::string&) const override {            \
      return std::make_unique<xName>(this->name());                                     \
    }                                                                                   \
  };                                                                                    \
                                                                                        \
  using _X_CALLBACK(xName) = ::cloe::DirectCallback<xName>;

/**
 * Macro DEFINE_SIMPLE_ACTION defines an action that has only a single state
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
 * DEFINE_SIMPLE_ACTION(Abort, "abort", "abort simulation", Simulation, {
 *   ptr_->abort();
 * });
 * ```
 *
 * This will define the following classes for us:
 *
 * - `Abort`
 * - `AbortFactory`
 *
 * This event can be registered with the `register_event` helper function.
 */
#define DEFINE_SIMPLE_ACTION(xName, xname, xdescription, xState, xOperatorBlock)                \
  class xName : public ::cloe::Action {                                                         \
   public:                                                                                      \
    xName(const std::string& name, xState* ptr) : ::cloe::Action(name), ptr_(ptr) {}            \
    ::cloe::ActionPtr clone() const override { return std::make_unique<xName>(name(), ptr_); }  \
    void operator()(const ::cloe::Sync&, ::cloe::TriggerRegistrar&) override { xOperatorBlock } \
    void to_json(::cloe::Json&) const override {}                                               \
                                                                                                \
   private:                                                                                     \
    xState* ptr_;                                                                               \
  };                                                                                            \
                                                                                                \
  class _X_FACTORY(xName) : public ::cloe::ActionFactory {                                      \
   public:                                                                                      \
    using ActionType = xName;                                                                   \
                                                                                                \
    _X_FACTORY(xName)(xState * ptr) : ::cloe::ActionFactory(xname, xdescription), ptr_(ptr) {}  \
                                                                                                \
    ::cloe::ActionPtr make(const ::cloe::Conf&) const override {                                \
      return std::make_unique<xName>(name(), ptr_);                                             \
    }                                                                                           \
                                                                                                \
    ::cloe::ActionPtr make(const std::string&) const override {                                 \
      return std::make_unique<xName>(name(), ptr_);                                             \
    }                                                                                           \
                                                                                                \
   private:                                                                                     \
    xState* ptr_;                                                                               \
  };

#endif  // CLOE_TRIGGER_MACROS_HPP_
