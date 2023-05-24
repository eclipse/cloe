/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file cloe/data_broker.hpp
 *
 * Classes:
 *   DataBroker:
 *     A central registry for (type erased) signals.
 *
 *   Signal:
 *     A type-erased abstraction of a signal.
 *
 *   BasicContainer<T>
 *     An optional container storing the value of a signal and abstracting the interaction with Signal.
 *
 * Background:
 *   Real world simulations utilize manifolds of signals (variables) and coresponding types.
 *   Still the very basic problem around a single signal (variable) boils down to CRUD
 *   - declaring a signal (Create)
 *   - reading its value / receiving value-changed events (Read)
 *   - writing its value / triggering value-changed events (Update)
 *   - NOT NEEDED: undeclaring a signal (Delete)
 *
 * Concept:
 *   Abstract CRU(D) operations by registering signals in a uniform way by their name.
 */

#pragma once
#ifndef CLOE_DATA_BROKER_HPP_
#define CLOE_DATA_BROKER_HPP_

#include <any>
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <fmt/format.h>

namespace cloe {

namespace databroker {

/**
 * Maps a predicate to type T or type int.
 * \returns T, if condition is true
 *          int, otherwise
 */
template <typename T, bool condition>
struct type_t_or_int_if {
  using type = T;
};

template <typename T>
struct type_t_or_int_if<T, false> {
  using type = int;
};

/**
 * Predicate which determines whether a type is incompatible with the data
 * broker.
 */
template <typename T>
constexpr bool is_incompatible_type_v = std::is_void_v<T> || std::is_reference_v<T>;

/**
 * Determines a datatype which is compatible with the data broker, derived from
 * the template type.
 *
 * \returns T, if T is compatible with the data broker;
 *          int, otherwise
 * \note Purpose is to suppress irrelevant compiler errors, in case of
 *       incompatible data types
 */
template <typename T>
using compatible_base_t = typename type_t_or_int_if<T, !is_incompatible_type_v<T>>::type;

/**
 * Argument-/Return-Type of signal related functions
 */
template <typename T>
using signal_type_cref_t = databroker::compatible_base_t<T> const&;

/**
 * Type of event function, which is called when the value of a signal changed
 */
template <typename T>
using on_value_changed_callback_t = std::function<void(signal_type_cref_t<T>)>;

/**
 * Abstract event implementation.
 *
 * \tparam TArgs Arguments of the event handler function
 * \note: Design-Goals:
 * - Design-#1: Unsubscribing from an event is not intended
 */
template <typename... TArgs>
class Event {
 public:
  using EventHandler = std::function<void(TArgs...)>;

 private:
  std::vector<EventHandler> eventhandlers_{};

 public:
  /**
   * Add an event handler to this event.
   */
  void add(EventHandler handler) { eventhandlers_.emplace_back(std::move(handler)); }

  /**
   * Return the number of event handlers subscribed to this event.
   */
  std::size_t count() const { return eventhandlers_.size(); }

  /**
   * Raise this event.
   * \param args Parameters of this event
   */
  void raise(TArgs&&... args) const {
    for (const auto& eventhandler : eventhandlers_) {
      try {
        eventhandler(std::forward<decltype(args)>(args)...);
      } catch (...) {
        throw;
      }
    }
  }
};

}  // namespace databroker

// Forward declarations:
template <typename T>
class BasicContainer;
class Signal;
class DataBroker;

/**
 * Assert that the type-argument is compatible with the data broker.
 */
template <typename T>
constexpr void assert_static_type() {
  static_assert(databroker::is_incompatible_type_v<T> == false,
                "Incompatible-Datatype-Error.\n"
                "\n"
                "Please find the offending LOC in above line 'require from here'.\n"
                "\n"
                "Explanation/Reasoning:\n"
                "- references & void are fundamentally incompatible");
}

template <typename T>
using Container = BasicContainer<databroker::compatible_base_t<T>>;

template <typename T>
class BasicContainer {
 public:
  using value_type = databroker::compatible_base_t<T>;

 private:
  /**
   * Access-token for regulating API access (public -> private)
   */
  struct access_token {
    explicit access_token(int){};
  };

  value_type value_{};
  databroker::on_value_changed_callback_t<value_type> on_value_changed_{};
  Signal* signal_{};

 public:
  BasicContainer(Signal* signal,
                 databroker::on_value_changed_callback_t<value_type>
                     on_value_changed,
                 access_token)
      : value_(), on_value_changed_(std::move(on_value_changed)), signal_(signal) {
    update_accessor_functions(this);
  }
  BasicContainer(const BasicContainer&) = delete;
  BasicContainer(BasicContainer<value_type>&& source)
      : value_(std::move(source.value_))
      , on_value_changed_(std::move(source.on_value_changed_))
      , signal_(std::move(source.signal_)) {
    update_accessor_functions(this);
  }

  ~BasicContainer() { update_accessor_functions(nullptr); }
  BasicContainer& operator=(const BasicContainer&) = delete;
  BasicContainer& operator=(BasicContainer&&) = default;
  BasicContainer& operator=(databroker::signal_type_cref_t<T> value) {
    value_ = value;
    if (on_value_changed_) {
      on_value_changed_(value_);
    }
    return *this;
  }

  const value_type& value() const { return value_; }
  value_type& value() { return value_; }
  void set_value(databroker::signal_type_cref_t<T> value) { *this = value; }

  bool has_subscriber() const;
  std::size_t subscriber_count() const;

  // mimic std::optional
  constexpr const value_type* operator->() const noexcept { return &value_; }
  constexpr value_type* operator->() noexcept { return &value_; }
  constexpr const value_type& operator*() const noexcept { return value_; }
  constexpr value_type& operator*() noexcept { return value_; }

 private:
  void update_accessor_functions(BasicContainer* container);

  friend class Signal;
};

// Compare two BasicContainer

template <typename T, typename U>
constexpr bool operator==(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs == *rhs;
}
template <typename T, typename U>
constexpr bool operator!=(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs != *rhs;
}
template <typename T, typename U>
constexpr bool operator<(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs < *rhs;
}
template <typename T, typename U>
constexpr bool operator<=(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs <= *rhs;
}
template <typename T, typename U>
constexpr bool operator>(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs > *rhs;
}
template <typename T, typename U>
constexpr bool operator>=(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs >= *rhs;
}

// Compare BasicContainer with a value

template <typename T, typename U>
constexpr bool operator==(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs == rhs;
}
template <typename T, typename U>
constexpr bool operator==(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs == *rhs;
}
template <typename T, typename U>
constexpr bool operator!=(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs != rhs;
}
template <typename T, typename U>
constexpr bool operator!=(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs != *rhs;
}
template <typename T, typename U>
constexpr bool operator<(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs < rhs;
}
template <typename T, typename U>
constexpr bool operator<(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs < *rhs;
}
template <typename T, typename U>
constexpr bool operator<=(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs <= rhs;
}
template <typename T, typename U>
constexpr bool operator<=(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs <= *rhs;
}
template <typename T, typename U>
constexpr bool operator>(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs > rhs;
}
template <typename T, typename U>
constexpr bool operator>(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs > *rhs;
}
template <typename T, typename U>
constexpr bool operator>=(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs >= rhs;
}
template <typename T, typename U>
constexpr bool operator>=(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs >= *rhs;
}

using SignalPtr = std::shared_ptr<Signal>;

/**
 * Signal-Descriptor
 *
 * \note: Design-Goals:
 * - Design-#1: The class shall expose a uniform interface (via type-erasure)
 * - Design-#2: The class shall be constructible via external helpers
 * - Design-#3: CppCoreGuidelines CP-1 does not apply. See also: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rconc-multi
 * - Design-#4: Templates shall be instantiated explicitly
 * \note: Implementation-Notes:
 * - Implementation-#1: Type specific aspects are implemented in templates (Design-#1)
 * - Implementation-#2: Objects are created via a factory-method (to prevent instances of incomplete initialization)
 * - Implementation-#3: An access-token is used to have a public c'tor which in fact shall be inaccesible (Design-#2).
 */
class Signal {
 private:
  /**
   * Access-token for regulating API access (public -> private).
   */
  struct access_token {
    explicit access_token(int){};
  };

  /**
   * Getter-function types
   *
   * Note:
   * When implementing this function with lamdas (e.g. set_getter<>() )
   * take care to explicitly define the return-type.
   *
   * Example:
   * []() -> const int& { return value; }
   *
   * Undefined behaviour:
   * []()     { return value; }
   *      ^^^ No return type specified
   */
  template <typename T>
  using typed_get_value_function_t = std::function<databroker::signal_type_cref_t<T>()>;
  using type_erased_get_value_function_t = std::any;

  /**
   * Setter-function types
   */
  template <typename T>
  using typed_set_value_function_t = std::function<void(databroker::signal_type_cref_t<T>)>;
  using type_erased_set_value_function_t = std::any;
  /**
   * Event types
   */
  template <typename T>
  using typed_value_changed_event_t = databroker::Event<databroker::signal_type_cref_t<T>>;
  using type_erased_value_changed_event_t = std::any;
  /**
   * Event trigger-function types
   */
  template <typename T>
  using typed_on_value_change_event_function_t =
      std::function<void(databroker::signal_type_cref_t<T>)>;
  using type_erased_on_value_change_event_function_t = std::any;

  /// Name(s) of the signal
  std::vector<std::string> names_{};
  /// std::type_info of the signal
  const std::type_info* type_{nullptr};
  /// getter-function
  type_erased_get_value_function_t get_value_{};
  /// setter-function
  type_erased_set_value_function_t set_value_{};
  /// Event which gets raised when the signal value changes
  type_erased_value_changed_event_t value_changed_event_{};
  /// Triggers the value-changed event
  type_erased_on_value_change_event_function_t on_value_changed_{};
  /// std::function returning the count of event subscribers
  std::function<std::size_t()> subscriber_count_{};

  /**
   * Private default c'tor
   * \note: Implementation-#2: The class shall be created only via a factory-method
   */
  Signal() = default;

 public:
  /**
   * Public c'tor, accessible only via private access-token
   * \note: Design-#1: The class shall be constructible via external helpers
   */
  explicit Signal(access_token) : Signal() {}
  Signal(const Signal&) = delete;
  Signal(Signal&&) = default;
  virtual ~Signal() = default;
  Signal& operator=(const Signal&) = delete;
  Signal& operator=(Signal&&) = default;

  /**
   * Return the type info for the signal.
   */
  constexpr const std::type_info* type() const { return type_; }

 private:
  /**
   * Validate that the stored signal type matches the template type.
   *
   * \tparam T Expected type of the signal
   */
  template <typename T>
  constexpr void assert_dynamic_type() const {
    const std::type_info* static_type = &typeid(T);
    const std::type_info* dynamic_type = type();
    if ((dynamic_type == nullptr) || (!(*dynamic_type == *static_type))) {
      throw std::logic_error(
          fmt::format("mismatch between dynamic-/actual-type and static-/requested-type; "
                      "signal type: {}, requested type: {}",
                      dynamic_type != nullptr ? dynamic_type->name() : "", static_type->name()));
    }
  }

 public:
  /**
   * Return the getter function of the signal.
   *
   * \tparam T Type of the signal
   * \return Getter function of the signal
   *
   * Example:
   * ```
   * Signal s = ...;
   * const typed_get_value_function_t<int>* f = s.getter<int>();
   * int v = (*f)();
   * ```
   */
  template <typename T>
  const typed_get_value_function_t<T>* getter() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    const typed_get_value_function_t<T>* get_value_fn =
        std::any_cast<typed_get_value_function_t<T>>(&get_value_);
    return get_value_fn;
  }

  /**
   * Set the getter function of the signal.
   *
   * \tparam T Type of the signal
   * \param get_value Getter function of the signal
   *
   * Example:
   * ```
   * Signal s = ...;
   * s.set_getter<type>([&]() -> const type & { return value; });
   * ```
   * Usage Note: When using lambdas, the explicit return-type definition is important!
   *
   * Undefined behaviour:
   * ```
   * Signal s = ...;
   * s.set_getter<type>([&]()     { return value; });
   *                          ^^^ No return type specified, type conversion rules apply
   * ```
   */
  template <typename T>
  void set_getter(typed_get_value_function_t<T> get_value_fn) {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    get_value_ = std::move(get_value_fn);
  }

  /**
   * Return the current value of the signal.
   *
   * \tparam T Type of the signal
   * \return databroker::signal_type_cref_t<T>, Current value of the signal
   * \note databroker::compatible_base_t<T> == T, if the method compiles
   */
  template <typename T>
  databroker::signal_type_cref_t<T> value() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const typed_get_value_function_t<T>* getter_fn = getter<compatible_type>();
    if (getter_fn && (*getter_fn)) {
      databroker::signal_type_cref_t<T> value = getter_fn->operator()();
      return value;
    }
    throw std::logic_error(
        fmt::format("unable to get value for signal without getter-function: {}", names_.front()));
  }

  /**
   * Return the getter function of the signal.
   *
   * \tparam T Type of the signal
   * \return const typed_set_value_function_t<T>*, Getter function of the signal
   */
  template <typename T>
  const typed_set_value_function_t<T>* setter() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    const typed_set_value_function_t<T>* set_value_fn =
        std::any_cast<typed_set_value_function_t<T>>(&set_value_);
    return set_value_fn;
  }

  /**
   * Set the setter function of the signal.
   *
   * \tparam T Type of the signal
   * \param set_value Getter function of the signal
   */
  template <typename T>
  void set_setter(typed_set_value_function_t<T> set_value_fn) {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    set_value_ = std::move(set_value_fn);
  }

  /**
   * Set the value of the signal.
   *
   * \tparam T Type of the signal
   * \param value Value of the signal
   */
  template <typename T>
  void set_value(databroker::signal_type_cref_t<T> value) const {
    assert_static_type<T>();
    assert_dynamic_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const typed_set_value_function_t<T>* setter_fn = setter<compatible_type>();
    if (setter_fn && *setter_fn) {
      setter_fn->operator()(value);
      return;
    }
    throw std::logic_error(
        fmt::format("unable to set value for signal without setter-function: {}", names_.front()));
  }

  /**
   * Return the trigger function for the value_changed event.
   *
   * \tparam T Type of the signal
   * \return Trigger function for raising the value_changed event
   */
  template <typename T>
  const typed_on_value_change_event_function_t<T>& trigger() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    const typed_on_value_change_event_function_t<T>* trigger_fn =
        std::any_cast<typed_on_value_change_event_function_t<T>>(&on_value_changed_);
    assert(trigger_fn);
    return *trigger_fn;
  }

 private:
  /**
   * Unpack the event to Event<const T*> and provide it to the caller.
   *
   * \tparam T Type of the event
   * \param callback Caller function which accepts the unpacked event
   * \note: In case type T and actual type do not match an exception is thrown
   */
  template <typename T>
  void subscribe_impl(
      const std::function<void(databroker::Event<databroker::signal_type_cref_t<T>>&)>& callback) {
    typed_value_changed_event_t<T>* value_changed_event =
        std::any_cast<typed_value_changed_event_t<T>>(&value_changed_event_);
    if (callback) {
      assert(value_changed_event);
      callback(*value_changed_event);
    }
  }

 public:
  /**
   * Subscribe to value-changed events.
   *
   * \tparam T Type of the signal
   * \param callback event-function which will be called when the value changed
   */
  template <typename T>
  void subscribe(databroker::on_value_changed_callback_t<T> callback) {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    subscribe_impl<T>(
        [callback = std::move(callback)](typed_value_changed_event_t<T>& value_changed_event) {
          value_changed_event.add(std::move(callback));
        });
  }

  /**
   * Return the count of subscribers to the value_changed event.
   *
   * \return size_t Count of subscribers to the value_changed event
   */
  std::size_t subscriber_count() const { return subscriber_count_(); }

  /**
   * Indicate whether the value_changed event has subscribers.
   *
   * \return bool True if the value_changed event has subscribers, false otherwise
   */
  bool has_subscriber() const { return subscriber_count() > 0; }

  /**
   * Return the list of names assigned to the signal.
   *
   * \return List of names assigned to the signal
   */
  const std::vector<std::string>& names() const { return names_; }

  /**
   * Return the first assigned name of the signal.
   *
   * \return First name of the signal
   */
  const std::string& name() const {
    if (!names_.size()) {
      throw std::logic_error("signal does not have a name");
    }
    return names_.front();
  }

  /**
   * Return the first assigned name of the signal.
   *
   * \return First name of the signal
   */
  std::string name_or(std::string def) const {
    if (!names_.size()) {
      return def;
    }
    return names_.front();
  }

  /**
   * Add a name of the signal.
   *
   * \param name Name of the signal
   */
  void add_name(std::string_view name) { names_.emplace_back(name); }

 private:
  /**
   * Factory for Signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return owning unique pointer to Signal
   * \note: Design-Note #1 reasons the existance of this factory
   */
  template <typename T>
  static std::unique_ptr<Signal> make() {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    auto signal = std::make_unique<Signal>(access_token(0));
    signal->initialize<compatible_type>();
    return signal;
  }

  /**
   * Create the container for a signal.
   *
   * \tparam T Type of the signal
   * \return BasicContainer for the signal
   * \note Design-#4: Templates shall be instantiated explicitly
   */
  template <typename T>
  BasicContainer<T> create_container() {
    assert_static_type<T>();

    typed_value_changed_event_t<T>* value_changed_event =
        std::any_cast<typed_value_changed_event_t<T>>(&value_changed_event_);
    // Create container
    BasicContainer<T> result = BasicContainer<T>(
        this,
        [value_changed_event](databroker::signal_type_cref_t<T> value) {
          value_changed_event->raise(std::move(value));
        },
        typename BasicContainer<T>::access_token(0));
    return result;
  }

 public:
 private:
  template <typename T>
  void initialize() {
    assert_static_type<T>();

    type_ = &typeid(T);
    // Create event
    value_changed_event_ = typed_value_changed_event_t<T>();
    typed_value_changed_event_t<T>* value_changed_event =
        &std::any_cast<typed_value_changed_event_t<T>&>(value_changed_event_);
    // Create event-trigger
    typed_on_value_change_event_function_t<T> on_value_changed =
        [value_changed_event](databroker::signal_type_cref_t<T> value) {
          value_changed_event->raise(std::move(value));
        };
    on_value_changed_ = on_value_changed;
    // Create subscriber_count function
    subscriber_count_ = [value_changed_event]() { return value_changed_event->count(); };
  }

  template <typename T>
  friend class BasicContainer;
  friend class DataBroker;
};

template <typename T>
void BasicContainer<T>::update_accessor_functions(BasicContainer* container) {
  if (signal_) {
    // Create getter-function
    if (container) {
      signal_->template set_getter<T>(
          [container]() -> databroker::signal_type_cref_t<T> { return container->value(); });
      signal_->template set_setter<T>(
          [container](databroker::signal_type_cref_t<T> value) { container->set_value(value); });
    } else {
      signal_->template set_getter<T>(Signal::typed_get_value_function_t<T>());
      signal_->template set_setter<T>(Signal::typed_set_value_function_t<T>());
    }
  }
}

template <typename T>
bool BasicContainer<T>::has_subscriber() const {
  return signal_ ? signal_->has_subscriber() : false;
}

template <typename T>
std::size_t BasicContainer<T>::subscriber_count() const {
  return signal_ ? signal_->subscriber_count() : 0;
}

/**
 * Registry for type-erased signals.
 */
class DataBroker {
 public:
  using SignalContainer = std::map<std::string, SignalPtr, std::less<>>;

 private:
  SignalContainer signals_{};

 public:
  DataBroker() = default;
  DataBroker(const DataBroker&) = delete;
  DataBroker(DataBroker&&) = delete;
  ~DataBroker() = default;
  DataBroker& operator=(const DataBroker&) = delete;
  DataBroker& operator=(DataBroker&&) = delete;

 public:
  /**
   * Return the signal with the given name.
   *
   * \param name Name of the signal
   * \return Signal with the given name
   */
  [[nodiscard]] const SignalContainer::const_iterator operator[](std::string_view name) const {
    SignalContainer::const_iterator iter = signals_.find(name);
    return iter;
  }

  /**
   * Return the signal with the given name.
   *
   * \param name Name of the signal
   * \return Signal with the given name
   */
  [[nodiscard]] SignalContainer::iterator operator[](std::string_view name) {
    SignalContainer::iterator iter = signals_.find(name);
    return iter;
  }

  /**
   * Give an existing signal an alias.
   *
   * \param signal Signal to be named
   * \param new_name New name of the signal
   * \return Pointer to the signal
   * \note If an exception is thrown by any operation, the aliasing has no effect.
   */
  SignalPtr alias(SignalPtr signal, std::string_view new_name) {
    if (new_name.empty()) {
      throw std::invalid_argument(
          fmt::format("alias for signal must not be empty: {}", signal->name_or("<unnamed>")));
    }

    // Mutate signals
    std::pair<SignalContainer::iterator, bool> inserted =
        signals_.try_emplace(std::string(new_name), std::move(signal));
    if (!inserted.second) {
      throw std::out_of_range(fmt::format("cannot alias signal '{}' to '{}': name already exists",
                                          signal->name_or("<unnamed>"), new_name));
    }
    // signals mutated, there is a liability in case of exceptions
    try {
      SignalPtr result = inserted.first->second;
      result->add_name(new_name);
      return result;
    } catch (...) {
      // fullfill exception guarantee (map.erase(iter) does not throw)
      signals_.erase(inserted.first);
      throw;
    }
  }

  /**
   * Give an existing signal a (new) name.
   *
   * \param old_name Name of the existing signal
   * \param new_name New name of the signal
   * \param f flag_type flags used to guide the interpretation of the character sequence as a regular expression
   * \return Pointer to the signal
   * \note If an exception is thrown by any operation, the aliasing has no effect.
   */
  SignalPtr alias(std::string_view old_name, std::string_view new_name,
                  std::regex::flag_type f = std::regex_constants::ECMAScript) {
    std::regex regex = std::regex(std::string(old_name), f);
    auto it1 = signals().begin();
    auto it2 = signals().begin();
    auto end = signals().end();
    const auto predicate = [&](const auto& item) -> bool {
      std::smatch match;
      return std::regex_match(item.first, match, regex);
    };
    it1 = (it1 != end) ? std::find_if(it1, end, predicate) : end;
    it2 = (it1 != end) ? std::find_if(std::next(it1), end, predicate) : end;
    if (it2 != end) {
      throw std::out_of_range(
          fmt::format("regex pattern matches multiple signals: '{}'; matches: '{}', '{}'", old_name,
                      it1->first, it2->first));
    }
    if (it1 == end) {
      throw std::out_of_range(fmt::format("regex pattern matches zero signals: {}", old_name));
      return nullptr;
    }
    SignalPtr result = alias(it1->second, new_name);
    return result;
  }

  /**
   * Declare a new signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return Pointer to the specified signal
   */
  template <typename T>
  SignalPtr declare(std::string_view new_name) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    SignalPtr signal = Signal::make<compatible_type>();
    alias(signal, new_name);
    return signal;
  }

  /**
   * Declare a new signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return Container<T> storing the signal value
   */
  template <typename T>
  [[nodiscard]] Container<databroker::compatible_base_t<T>> implement(std::string_view new_name) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    SignalPtr signal = declare<compatible_type>(new_name);
    Container<compatible_type> container = signal->create_container<compatible_type>();
    return container;
  }

  /**
   * Return the signal with the given name.
   *
   * \param name Name of the signal
   * \return Signal with the given name
   */
  [[nodiscard]] SignalPtr signal(std::string_view name) const {
    auto iter = (*this)[name];
    if (iter != signals_.end()) {
      return iter->second;
    }
    throw std::out_of_range(fmt::format("signal not found: {}", name));
  }

  /**
   * Return the signal with the given name.
   *
   * \param name Name of the signal
   * \return Signal with the given name
   */
  [[nodiscard]] SignalPtr signal(std::string_view name) {
    auto iter = (*this)[name];
    if (iter != signals_.end()) {
      return iter->second;
    }
    throw std::out_of_range(fmt::format("signal not found: {}", name));
  }

  /**
   * Return all signals.
   */
  [[nodiscard]] const SignalContainer& signals() const { return signals_; }

  /**
   * Return all signals.
   */
  [[nodiscard]] SignalContainer& signals() {
    // DRY
    return const_cast<SignalContainer&>(const_cast<const DataBroker*>(this)->signals());
  }

  /**
   * Subscribe to value-changed events.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \param callback event-function which will be called when the value changed
   */
  template <typename T>
  void subscribe(std::string_view name, databroker::on_value_changed_callback_t<T> callback) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    signal(name)->template subscribe<compatible_type>(std::move(callback));
  }

  /**
   * Set the value of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \param value Value to be assigned to the signal
   */
  template <typename T>
  void set_value(std::string_view name, databroker::signal_type_cref_t<T> value) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    signal(name)->set_value<compatible_type>(value);
  }

  /**
   * Return the value of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return Pointer to the value of the signal
   * \note databroker::compatible_base_t<T> == T, if the function compiles
   */
  template <typename T>
  databroker::signal_type_cref_t<T> value(std::string_view name) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    return signal(name)->value<compatible_type>();
  }

  /**
   * Return the getter-function of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return getter-function of the signal
   */
  template <typename T>
  const Signal::typed_get_value_function_t<T>& getter(std::string_view name) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const Signal::typed_get_value_function_t<T>* getter_fn =
        signal(name)->getter<compatible_type>();
    if (!getter_fn) {
      throw std::logic_error(fmt::format("getter for signal not provided: {}", name));
    }
    return *getter_fn;
  }

  /**
   * Return the setter-function of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return const Signal::typed_set_value_function_t<T>&, setter-function of the signal
   */
  template <typename T>
  const Signal::typed_set_value_function_t<T>& setter(std::string_view name) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const Signal::typed_set_value_function_t<T>* setter_fn =
        signal(name)->setter<compatible_type>();
    if (!setter_fn) {
      throw std::logic_error(fmt::format("setter for signal not provided: {}", name));
    }
    return *setter_fn;
  }
};

// Design-Goals:
// -g1: Prepare for mass usage of Signals, Containers & Databroker
//      ==> Allow instantiation of all templates for relevant datatypes
// -g2: Pre-Instantiate these templates for datatypes which are already known to be relevant

#define CLOE_DATABROKER_TEMPLATE_INSTANTIATION(elem)                                               \
  extern template class BasicContainer<elem>;                                                      \
  extern template const Signal::typed_get_value_function_t<elem>* Signal::getter<elem>() const;    \
  extern template void Signal::set_getter<elem>(Signal::typed_get_value_function_t<elem>);         \
  extern template databroker::signal_type_cref_t<elem> Signal::value<elem>() const;                \
  extern template const Signal::typed_set_value_function_t<elem>* Signal::setter<elem>() const;    \
  extern template void Signal::set_setter<elem>(Signal::typed_set_value_function_t<elem>);         \
  extern template void Signal::set_value<elem>(databroker::signal_type_cref_t<elem> value) const;  \
  extern template const Signal::typed_on_value_change_event_function_t<elem>&                      \
  Signal::trigger<elem>() const;                                                                   \
  extern template void Signal::subscribe_impl<elem>(                                               \
      const std::function<void(databroker::Event<databroker::signal_type_cref_t<elem>>&)>&);       \
  extern template void Signal::subscribe<elem>(databroker::on_value_changed_callback_t<elem>);     \
  extern template std::unique_ptr<Signal> Signal::make<elem>();                                    \
  extern template BasicContainer<elem> Signal::create_container<elem>();                           \
  extern template void Signal::initialize<elem>();                                                 \
  extern template SignalPtr DataBroker::declare<elem>(std::string_view);                           \
  extern template BasicContainer<elem> DataBroker::implement<elem>(std::string_view);              \
  extern template void DataBroker::subscribe<elem>(std::string_view,                               \
                                                   databroker::on_value_changed_callback_t<elem>); \
  extern template databroker::signal_type_cref_t<elem> DataBroker::value<elem>(std::string_view);  \
  extern template const Signal::typed_get_value_function_t<elem>& DataBroker::getter<elem>(        \
      std::string_view name);                                                                      \
  extern template void DataBroker::set_value<elem>(std::string_view,                               \
                                                   databroker::signal_type_cref_t<elem>);          \
  extern template const Signal::typed_set_value_function_t<elem>& DataBroker::setter<elem>(        \
      std::string_view name);

#define CLOE_DATABROKER_TEMPLATE_INSTANTATION_TYPES() \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(bool)        \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(int8_t)      \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(uint8_t)     \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(int16_t)     \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(uint16_t)    \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(int32_t)     \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(uint32_t)    \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(int64_t)     \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(uint64_t)    \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(float)       \
  CLOE_DATABROKER_TEMPLATE_INSTANTIATION(double)

CLOE_DATABROKER_TEMPLATE_INSTANTATION_TYPES()

// Goal g1 implies that the preprocessor macros are not undefined.

}  // namespace cloe

#endif  // CLOE_DATA_BROKER_HPP_
