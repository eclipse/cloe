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
 * \file cloe/trigger.hpp
 * \see  cloe/trigger.cpp
 * \see  cloe/registrar.hpp
 * \see  cloe/trigger/example_actions.hpp
 * \see  cloe/trigger/set_action.hpp
 * \see  cloe/trigger/nil_event.hpp
 */

#pragma once
#ifndef CLOE_TRIGGER_HPP_
#define CLOE_TRIGGER_HPP_

#include <memory>   // for unique_ptr<>, shared_ptr<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <cloe/core.hpp>    // for Schema, Json, Duration, Error
#include <cloe/entity.hpp>  // for Entity
#include <fable/enum.hpp>   // for ENUM_SERIALIZATION

namespace cloe {

// Forward declarations:
class Sync;  // from cloe/sync.hpp

// Forward declarations for Trigger:
class Event;
using EventPtr = std::unique_ptr<Event>;
using EventPtrs = std::vector<EventPtr>;
class Action;
using ActionPtr = std::unique_ptr<Action>;
using ActionPtrs = std::vector<ActionPtr>;
class Trigger;
using TriggerPtr = std::unique_ptr<Trigger>;
using TriggerPtrs = std::vector<TriggerPtr>;

/**
 * TriggerError is thrown whenever an error relating to triggers occurs.
 * This includes parsing, insertion, and execution of triggers.
 */
class TriggerError : public Error {
 public:
  using Error::Error;
  virtual ~TriggerError() noexcept = default;
};

/**
 * TriggerInvalid indicates that the trigger cannot be inserted because it
 * is somehow invalid.
 *
 * This could be because a section is missing, or that the input values are
 * inappropriate for the factory of an event or action.
 */
class TriggerInvalid : public TriggerError {
 public:
  TriggerInvalid(const Conf& c, const std::string& what) : TriggerError(what), conf_(c) {}
  virtual ~TriggerInvalid() noexcept = default;

  const Conf& conf() const { return conf_; }

 private:
  Conf conf_;
};

/**
 * InlineSchema describes the schema of the inline format.
 */
struct InlineSchema {
 public:
  /**
   * Construct an implicit inline schema if enabled is true, i.e., one where
   * only the name itself is sufficient.
   *
   * If possible, it is recommended to use InlineSchema(std::string&&) instead.
   * This constructor is primarily useful when you want to explicitly disable
   * an inline schema.
   */
  explicit InlineSchema(bool enabled) : required_(!enabled) {}

  /**
   * Construct an implicit inline schema with the given description.
   */
  explicit InlineSchema(std::string&& desc) : required_(false), desc_(std::move(desc)) {}

  /**
   * Construct an inline schema that takes a particular primitive type.
   *
   * The type may not be null, object, or array.
   */
  InlineSchema(std::string&& desc, JsonType type, bool required = true);

  /**
   * Construct an inline schema that takes a string with the given format.
   *
   * This is only needed when further parsing of the string is applied.
   * The format should describe the string that will directly parsed by the
   * TriggerFactory::make(const std::string&) method.
   *
   * The string should roughly follow the (extended) Backus-Naur form, as is
   * known from many usage strings:
   *
   *    stop                A single string "stop"
   *    <button>            A button identifier, whatever that is
   *    [!]<button>         A button identifier, prefixed optionally with "!"
   *    <string>,<float>    A string, comma, and float
   *    <string>[,<float>]  A string optionally followed by a comma and float
   *    <id>[,...]          An array of ids, without a terminating comma
   *
   * It's generally not possible to be readable and exact at the same time.
   * There is some ambiguity that we just need to live with. Use some common
   * sense here and see what is in use. Embedded quotes may help with the
   * ambiguity but should be used sparingly (for now, as usage is often
   * directly read from JSON).
   */
  InlineSchema(std::string&& desc, std::string&& format, bool required = true)
      : type_(JsonType::string)
      , required_(required)
      , usage_(std::move(format))
      , desc_(std::move(desc)) {}

  /**
   * Return the description of the inline schema.
   */
  const std::string& description() const { return desc_; }

  /**
   * Return the argument type of the inline schema.
   *
   * If the type is JsonType::null, then the schema takes no arguments,
   * as is the case for many events, such as "start" or "stop".
   */
  JsonType type() const { return type_; }

  /**
   * Return whether the inline format can be used for this trigger.
   *
   * The inline format is considered disabled when the argument type is null
   * and yet required. This cannot after all be a useful call to make.
   */
  bool is_enabled() const { return !(required_ && type_ == JsonType::null); }

  /**
   * Return whether the single argument to the trigger is required.
   *
   * Note: When no arguments are accepted and the trigger is enabled,
   *       false is returned. Therefore,
   *          !is_required()
   *       is all that is needed to ascertain that the name is sufficient,
   *       such as in "start" or "next".
   */
  bool is_required() const { return required_; }

  /**
   * Return the usage of the inline schema.
   *
   * If not enabled, then the empty string "" is returned.
   * Otherwise, the return value takes one of the following forms:
   *
   *    NAME
   *    NAME=TYPE
   *    NAME[=TYPE]
   *    NAME=FORMAT
   *    NAME[=FORMAT]
   *
   * For example:
   *
   *    stop
   *    time=float
   *    next[=float]
   *
   * Note: The output of this method should not change given a certain name.
   *       It is thus a good candidate for caching should this datum be need,
   *       as the calculation is not particularly cheap.
   */
  std::string usage(const std::string& name) const;

 private:
  JsonType type_{JsonType::null};
  bool required_{true};
  std::string usage_{};
  std::string desc_{};
};

/**
 * TriggerSchema describes the schema of a trigger, acting as a usage generator
 * and validator for trigger actions and events.
 */
struct TriggerSchema {
 public:
  /**
   * Construct a TriggerSchema that describes a trigger with no parameters
   * and an implicit inline format.
   *
   * For example, the definition:
   *
   *    TriggerSchema("stop", "stop the simulation")
   *
   * will allow the following forms of initialization:
   *
   *    "stop"
   *    {
   *      "name": "stop"
   *    }
   */
  TriggerSchema(const std::string& name, const std::string& desc)
      : name_(name), schema_(std::string(desc)), inline_(true) {}

  /**
   * Construct a TriggerSchema that describes a trigger with parameters
   * but no inline format.
   */
  TriggerSchema(const std::string& name, const std::string& desc,
                fable::schema::PropertyList<> props)
      : name_(name), schema_(std::string(desc), props), inline_(false) {}

  /**
   * Construct a TriggerSchema that describes a trigger with the given Schema
   * but no inline format.
   */
  TriggerSchema(const std::string& name, const std::string& desc, Schema&& s)
      : name_(name), schema_(std::move(s)), inline_(false) {
    schema_.set_description(desc);
  }

  /**
   * Construct a TriggerSchema that describes a trigger with parameters
   * and a specified inline format.
   */
  TriggerSchema(const std::string& name, const std::string& desc, InlineSchema&& usage,
                fable::schema::PropertyList<> props)
      : name_(name), schema_(std::string(desc), props), inline_(std::move(usage)) {}

  /**
   * Construct a TriggerSchema that describes a trigger with the given Schema
   * and a specified inline format.
   */
  TriggerSchema(const std::string& name, const std::string& desc, InlineSchema&& usage,
                Schema&& init)
      : name_(name), schema_(std::move(init)), inline_(std::move(usage)) {
    schema_.set_description(desc);
  }

  /**
   * Construct a TriggerSchema that describes a trigger with the given Schema
   * and a specified inline format.
   */
  TriggerSchema(const std::string& name, const std::string& desc, InlineSchema&& usage,
                const Schema& init)
      : name_(name), schema_(init), inline_(std::move(usage)) {
    schema_.set_description(desc);
  }

  const std::string& name() const { return name_; }
  const std::string& description() const { return schema_.description(); }
  std::string usage_inline() const { return inline_.usage(name_); }
  Json usage() const { return schema_.usage(); }
  Json json_schema() const;

 private:
  std::string name_;
  Schema schema_;
  InlineSchema inline_;
};

/**
 * TriggerFactory is a superclass for EventFactory and ActionFactory that
 * also accepts alternate inputs for configuration.
 *
 * This class should not be used directly; instead, new factories should base
 * off of EventFactory or ActionFactory, which inherit from AlternateFactory.
 *
 * It extends the Factory interface with the method `make(const std::string&)`;
 * this allows the creation of an event or a factory from a string instead of
 * full JSON object.
 *
 * The default schema is an implicit one; that is, the factory takes no
 * configuration but can be called with the name only. An inline format
 * is therefore also implicitely supported.
 */
template <typename T>
class TriggerFactory : public Entity {
 public:
  using Entity::Entity;
  virtual ~TriggerFactory() noexcept = default;

  /**
   * Return factory schema.
   *
   * This can be used to check input automatically or to derive help text
   * for all trigger events and actions dynamically.
   */
  virtual TriggerSchema schema() const { return TriggerSchema{this->name(), this->description()}; }

  /**
   * Return factory usage.
   */
  virtual Json json_schema() const { return this->schema().json_schema(); }

  /**
   * Create a new T based on the content of the input Conf.
   *
   * - This method may throw TriggerInvalid.
   */
  virtual std::unique_ptr<T> make(const Conf& c) const = 0;

  /**
   * Create a new T based on the content of the input string.
   *
   * The default implementation tries to make an instance with an empty JSON
   * object, which only works for actions and events that require no input.
   *
   * It is strongly recommended to implement this method by creating a Conf
   * object and passing that to the `make(const Conf&)` method. This allows
   * users to draw a strong parallel from string representation to JSON
   * representation.
   *
   * - This method may throw TriggerInvalid.
   */
  virtual std::unique_ptr<T> make(const std::string& s) const {
    if (s.empty()) {
      // If the string is empty, then we can probably just assume that a Conf
      // with nothing inside is as good as if it were the only one. Some
      // information does go lost this way though.
      auto c = Conf{
          {"name", this->name()},
      };
      return this->make(c);
    } else {
      throw TriggerInvalid(Conf{Json{s}}, "cannot create " + this->name() + " from '" + s + "'");
    }
  }
};

/**
 * Source is an enumeration of all possible Trigger origins.
 *
 * This enables reproducibility by allowing a simulation to ignore triggers
 * with certain sources, for example the web UI. When saving a history of
 * triggers, it also highlights triggers that are generated by other triggers.
 */
enum class Source {
  /// Triggers that originate from the filesystem, such as stack files.
  FILESYSTEM,

  /// Triggers that originate from the network API, such as JSON data.
  /// It may be useful to disable these to prevent dynamic interactions.
  NETWORK,

  /// Triggers that originate from models, such as a simulator binding.
  MODEL,

  /// Triggers that originate from triggers themselves.
  /// It may be necessary to disable these to prevent duplicate triggers.
  TRIGGER,

  /// Triggers that are instance of a sticky trigger.
  INSTANCE,
};

// clang-format off
ENUM_SERIALIZATION(Source, ({
  {Source::FILESYSTEM, "filesystem"},
  {Source::NETWORK, "network"},
  {Source::MODEL, "model"},
  {Source::TRIGGER, "trigger"},
  {Source::INSTANCE, "instance"},
}))
// clang-format on

/**
 * Return whether a source is considered transient.
 *
 * Transient sources are those where the trigger is generated as opposed to
 * originating from the user. This is an important distinction as generated
 * (i.e., transient) triggers should not be re-inserted for simulation
 * reproduction.
 */
inline bool source_is_transient(Source s) {
  return (s != Source::FILESYSTEM && s != Source::NETWORK);
}

/**
 * Trigger contains an event-action pair, that is executed when the event
 * is triggered.
 */
class Trigger {
 public:
  Trigger(const std::string& label, Source s, EventPtr&& e, ActionPtr&& a);
  TriggerPtr clone() const;

  const std::string& label() const { return label_; }
  Source source() const { return source_; }
  Duration since() const { return since_; }
  void set_since(Duration t) { since_ = t; }
  const Event& event() const { return *event_; }
  Event& event() { return *event_; }
  const Action& action() const { return *action_; }
  Action& action() { return *action_; }

  bool is_significant() const;
  bool is_transient() const { return source_is_transient(source_) || is_conceal(); }
  bool is_conceal() const { return conceal_; }
  void set_conceal(bool value = true);
  bool is_sticky() const { return sticky_; }
  void set_sticky(bool value = true);

  friend void to_json(Json& j, const Trigger& t);

 private:
  std::string label_;
  Source source_;
  EventPtr event_;
  ActionPtr action_;
  Duration since_{0};
  bool conceal_{false};
  bool sticky_{false};
};

/**
 * TriggerRegistrar is a registrar interface for creating Events, Actions, and
 * Triggers and inserting them into the simulation.
 */
class TriggerRegistrar {
 public:
  explicit TriggerRegistrar(Source s) : source_(s) {}
  virtual ~TriggerRegistrar() noexcept = default;

  virtual ActionPtr make_action(const Conf& c) const = 0;
  virtual EventPtr make_event(const Conf& c) const = 0;
  virtual TriggerPtr make_trigger(const Conf& c) const = 0;

  virtual void insert_trigger(const Conf& c) = 0;
  virtual void insert_trigger(TriggerPtr&& t) = 0;

  /**
   * Create and insert a Trigger with given label, event, and action.
   *
   * Example:
   *
   *     ActionPtr a = tr.make_action(action_conf);
   *     EventPtr e = tr.make_event(event_conf);
   *     tr.insert_trigger("My trigger", std::move(e), std::move(a));
   */
  void insert_trigger(const std::string& label, EventPtr&& e, ActionPtr&& a);

 protected:
  Source source_;
};

/**
 * Event represents the event/condition portion of a trigger, and is what
 * causes an action to be executed.
 *
 * It is created through an EventFactory, and registered with the simulation
 * through the Registrar interface.
 *
 * The primary identifying interface of an Event is through it's constructor,
 * where it receives its name, together with the `to_json(Json&)` method, where
 * any further state is represented. This allows a new identical Event to be
 * created.
 *
 * The actual triggering of an Event is achieved through the Callback
 * interface, in which Triggers with the corresponding event are contained.
 *
 * \see  cloe/registrar.hpp
 * \see  cloe/trigger/nil_event.hpp
 */
class Event : public Entity {
 public:
  using Entity::Entity;
  virtual ~Event() noexcept = default;

  /**
   * Clone this event with correct state information.
   *
   * This happens when a sticky trigger evaluates to true.
   */
  virtual EventPtr clone() const = 0;

  /**
   * Describe the event state so that the same event can be re-created through
   * it's JSON representation with the corresponding EventFactory.
   */
  virtual void to_json(Json&) const = 0;
  friend void to_json(Json& j, const Event& e);

 protected:
  Logger logger() const { return logger::get("cloe/event/" + name()); }
};

/**
 * An EventFactory parses a single JSON object or string into an Event.
 *
 * \see  cloe/registrar.hpp
 * \see  cloe/trigger/nil_event.hpp
 */
using EventFactory = TriggerFactory<Event>;
using EventFactoryPtr = std::unique_ptr<EventFactory>;

/**
 * Interface the trigger manager must provide for executing triggers.
 */
using CallbackExecuter = std::function<void(TriggerPtr&&, const Sync&)>;

/**
 * Callback provides the interface with which the global trigger manager,
 * which maintains a list of all available event and action factories,
 * can insert and execute triggers.
 *
 * A Callback subtype is created for each event type, and is passed to the
 * trigger manager when the event factory is registered. This allows the event
 * owner, which knows exactly when it should trigger all events, to pass
 * activated triggers to the event manager. This is done through the
 * `execute(TriggerPtr&&, const Sync&)` method.
 *
 * It is strongly recommended to use the `DirectCallback` template class
 * instead of rolling your own Callback class. If you do roll your own,
 * ensure that you do not execute triggers directly; rather use the provided
 * CallbackExecuter.
 *
 * \see  cloe/registrar.hpp
 */
class Callback {
 public:
  virtual ~Callback() noexcept = default;

  /**
   * Set the trigger executer function.
   */
  void set_executer(CallbackExecuter exe) { executer_ = exe; }

  /**
   * Place a trigger within the callback for storage.
   */
  virtual void emplace(TriggerPtr&& t, const Sync& s) = 0;

  /**
   * Return JSON representation of all contained triggers.
   */
  virtual void to_json(Json& j) const = 0;
  friend void to_json(Json& j, const Callback& c) { c.to_json(j); }

 protected:
  /**
   * Execute a trigger in the given sync context by passing it to the
   * executer.
   */
  void execute(TriggerPtr&& t, const Sync& s);

 private:
  CallbackExecuter executer_;
};

/**
 * AliasCallback allows for a single Callback instance to be used for multiple
 * event factories.
 *
 * This callback is not meant to be triggered!
 */
class AliasCallback : public Callback {
 public:
  explicit AliasCallback(std::shared_ptr<Callback> owner) : owner_(owner) {}
  virtual ~AliasCallback() noexcept = default;

  /**
   * Delegate emplacement to the underlying owner of triggers.
   */
  void emplace(TriggerPtr&& t, const Sync& s) override { owner_->emplace(std::move(t), s); }

  /**
   * JSON output is zero.
   */
  void to_json(Json&) const override {}

 private:
  std::shared_ptr<Callback> owner_;
};

/**
 * Action represents the action portion of a trigger, and is what is executed
 * when a trigger condition evaluates true.
 *
 * It is created through an ActionFactory, and registered with the simulation
 * through the Registrar interface.
 *
 * The primary identifying interface of an Action is through it's constructor,
 * where it receives its name, together with the `to_json(Json&)` method, where
 * any further state is represented. This allows a new identical Action to be
 * created.
 *
 * The execution is achived through the function operator of the Action.
 * Any futher state required within the Action must be passed through
 * the constructor of the Action.
 *
 * \see  cloe/trigger/example_actions.hpp
 */
class Action : public Entity {
 public:
  using Entity::Entity;
  virtual ~Action() noexcept = default;

  /**
   * Clone this action with correct state information.
   *
   * This happens when a sticky trigger evaluates to true.
   */
  virtual ActionPtr clone() const = 0;

  /**
   * Execute the action.
   */
  virtual void operator()(const Sync&, TriggerRegistrar&) = 0;

  /**
   * Return whether this action is a signficant action.
   *
   * All actions that have an effect on the result of a simulation are
   * significant. Actions that are not significant are:
   *
   *  realtime_factor
   *  log
   *
   * Insignificant actions are concealable, which removes them from the trigger
   * history. This must be explicitly specified when inserting a trigger.
   */
  virtual bool is_significant() const { return true; }

  /**
   * Describe the action state so that the same action can be re-created
   * through it's JSON representation with the corresponding ActionFactory.
   */
  virtual void to_json(Json& j) const = 0;
  friend void to_json(Json& j, const Action& a);

 protected:
  Logger logger() const { return logger::get("cloe/action/" + name()); }
};

/**
 * An ActionFactory parses a single JSON object or string into an action.
 *
 * \see  cloe/trigger/example_actions.hpp
 */
using ActionFactory = TriggerFactory<Action>;
using ActionFactoryPtr = std::unique_ptr<ActionFactory>;

}  // namespace cloe
#endif  // CLOE_TRIGGER_HPP_
