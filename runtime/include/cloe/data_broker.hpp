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

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

#include <fmt/format.h>

#include "databroker/data_broker_types.hpp"
#include "databroker/data_broker_binding.hpp"
#include "databroker/data_broker_container.hpp"

namespace cloe {

// Forward declarations:
class Signal;

using SignalPtr = std::shared_ptr<Signal>;

/**
  * TypedSignal decorates Signal with a specific datatype.
  */
template <typename T>
class TypedSignal {
 private:
  SignalPtr signal_;

 public:
  TypedSignal(SignalPtr signal) : signal_{signal} {}
  ~TypedSignal() = default;

  operator SignalPtr&() { return signal_; }
  operator const SignalPtr&() const { return signal_; }

  const T& value() const { return signal_->template value<T>(); }

  template <typename TSetter>
  void set_setter(TSetter setter) {
    signal_->template set_setter<T>(std::move(setter));
  }
};

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
  explicit DataBroker(databroker::DataBrokerBinding* binding) : binding_(binding) {}
  DataBroker(const DataBroker&) = delete;
  DataBroker(DataBroker&&) = delete;
  ~DataBroker() = default;
  DataBroker& operator=(const DataBroker&) = delete;
  DataBroker& operator=(DataBroker&&) = delete;

 private:
  databroker::DataBrokerBinding* binding_{};
 public:

  /**
    * \brief Binds a signal to the Lua-VM
    * \param signal_name Name of the signal
    * \param lua_name Name of the table/variable used in Lua
    * \note The bind-method needs to be invoked at least once (in total) to bring all signal bindings into effect
    */
  void bind_signal(std::string_view signal_name, std::string_view lua_name) {
    if(!binding_) {
      throw std::logic_error(
          "DataBroker: Binding a signal to another language must not happen "
          "before its context is initialized.");
    }
    SignalPtr signal = this->signal(signal_name);
    binding_->bind_signal(signal, signal_name, lua_name);
  }

  /**
   * \brief Binds a signal to the Lua-VM
   * \param signal_name Name of the signal
   * \note The bind-method needs to be invoked at least once (in total) to bring all signal bindings into effect
   */
  void bind_signal(std::string_view signal_name) { bind_signal(signal_name, signal_name); }

  void bind(std::string_view signals_name) { binding_->bind(signals_name); }

 public:
  /**
   * Return the signal with the given name.
   *
   * \param name Name of the signal
   * \return Signal with the given name
   */
  [[nodiscard]] SignalContainer::const_iterator operator[](std::string_view name) const {
    return signals_.find(name);
  }

  /**
   * Return the signal with the given name.
   *
   * \param name Name of the signal
   * \return Signal with the given name
   */
  [[nodiscard]] SignalContainer::iterator operator[](std::string_view name) {
    return signals_.find(name);
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
      // fulfill exception guarantee (map.erase(iter) does not throw)
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

    // binding_->declare(type_index);

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

    // binding_->template declare<compatible_type>();

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
  databroker::signal_type_cref_t<T> value(std::string_view name) const {
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
  const Signal::typed_get_value_function_t<T>& getter(std::string_view name) const {
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
   * Sets the getter-function of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \param getter_fn getter-function of the signal
   */
  template <typename T>
  void set_getter(std::string_view name, const Signal::typed_get_value_function_t<T>& getter_fn) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    signal(name)->set_getter<compatible_type>(getter_fn);
  }

  /**
   * Return the setter-function of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return const Signal::typed_set_value_function_t<T>&, setter-function of the signal
   */
  template <typename T>
  const Signal::typed_set_value_function_t<T>& setter(std::string_view name) const {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const Signal::typed_set_value_function_t<T>* setter_fn =
        signal(name)->setter<compatible_type>();
    if (!setter_fn) {
      throw std::logic_error(fmt::format("setter for signal not provided: {}", name));
    }
    return *setter_fn;
  }
  /**
   * Sets the setter-function of a signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \param getter_fn setter-function of the signal
   */
  template <typename T>
  void set_setter(std::string_view name, const Signal::typed_set_value_function_t<T>& setter_fn) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    signal(name)->set_setter<compatible_type>(setter_fn);
  }
};

namespace databroker {

struct DynamicName {
  static constexpr bool STATIC = false;

 private:
  std::string name_;

 public:
  DynamicName(std::string name) : name_{name} {}
  const std::string& name() const { return name_; }
};

template <const char* NAME>
struct StaticName {
  static constexpr bool STATIC = true;
  static constexpr const char* name() { return NAME; }
};

/**
  * SignalDescriptorBase implements a SignalDescriptor
  *
  * \tparam T Type of the signal
  * \tparam TNAME Name of the signal
  * \tparam bool true for names, false otherwise
  */
template <typename T, typename TNAME, bool = TNAME::STATIC>
struct SignalDescriptorBase {};

/**
  * SignalDescriptorBase implements a SignalDescriptor, specialization for statically determined signal names
  *
  * \tparam T Type of the signal
  * \tparam TNAME Name of the signal
  */
template <typename T, typename TNAME>
struct SignalDescriptorBase<T, TNAME, true> : public TNAME {
  using TNAME::name;
  using TNAME::TNAME;
  /**
   * Implements the signal
   *
   * \param db Instance of the DataBroker
   * \return Container<type>, the container of the signal
   */
  static auto implement(DataBroker& db) { return db.implement<T>(name()); }
  /**
   * Declares the signal
   *
   * \param db Instance of the DataBroker
   * \return TypeSignal<type>, the signal
   */
  static void declare(DataBroker& db) { db.declare<T>(name()); }
  /**
   * Returns the instance of a signal.
   *
   * \param db Instance of the DataBroker
   * \return TypedSignal<T>, instance of the signal
   */
  static auto signal(const DataBroker& db) { return TypedSignal<T>(db.signal(name())); }
  /**
   * Return the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return const Signal::typed_get_value_function_t<T>&, getter-function of the signal
   */
  static auto getter(const DataBroker& db) { return db.getter<T>(name()); }
  /**
   * Sets the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \param get_value_fn getter-function of the signal
   */
  static void set_getter(DataBroker& db, Signal::typed_get_value_function_t<T> get_value_fn) {
    db.set_getter<T>(name(), std::move(get_value_fn));
  }
  /**
   * Return the setter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return const Signal::typed_set_value_function_t<type>&, setter-function of the signal
   */
  static auto setter(const DataBroker& db) { return db.setter<T>(name()); }
  /**
   * Sets the setter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \param set_value_fn setter-function of the signal
   */
  static void set_setter(DataBroker& db, Signal::typed_set_value_function_t<T> set_value_fn) {
    db.set_setter<T>(name(), std::move(set_value_fn));
  }

  /**
   * Return the value of a signal.
   *
   * \param db Instance of the DataBroker
   * \return Pointer to the value of the signal, nullptr if the signal does not exist
   */
  static auto value(DataBroker& db) { return db.value<T>(name()); }
  /**
   * Set the value of a signal.
   *
   * \param db Instance of the DataBroker
   * \param value Value to be assigned to the signal
   */
  static auto set_value(DataBroker& db, const T& value) { db.set_value<T>(name(), value); }
};

/**
  * SignalDescriptorBase implements a SignalDescriptor, specialization for dynamically determined signal names
  *
  * \tparam T Type of the signal
  */
template <typename T, typename TNAME>
struct SignalDescriptorBase<T, TNAME, false> : public TNAME {
  using TNAME::name;
  using TNAME::TNAME;
  /**
   * Implements the signal
   *
   * \param db Instance of the DataBroker
   * \return Container<type>, the container of the signal
   */
  auto implement(DataBroker& db) { return db.implement<T>(name()); }
  /**
   * Declares the signal
   *
   * \param db Instance of the DataBroker
   * \return TypeSignal<type>, the signal
   */
  void declare(DataBroker& db) { db.declare<T>(name()); }
  /**
   * Returns the instance of a signal.
   *
   * \param db Instance of the DataBroker
   * \return TypedSignal<T>, instance of the signal
   */
  auto signal(const DataBroker& db) const { return TypedSignal<T>(db.signal(name())); }
  /**
   * Return the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return const Signal::typed_get_value_function_t<T>&, getter-function of the signal
   */
  auto getter(const DataBroker& db) const { return db.getter<T>(name()); }
  /**
   * Sets the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \param get_value_fn getter-function of the signal
   */
  void set_getter(DataBroker& db, Signal::typed_get_value_function_t<T> get_value_fn) {
    db.set_getter<T>(name(), std::move(get_value_fn));
  }
  /**
   * Return the setter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return const Signal::typed_set_value_function_t<type>&, setter-function of the signal
   */
  auto setter(const DataBroker& db) const { return db.setter<T>(name()); }
  /**
   * Sets the setter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \param set_value_fn setter-function of the signal
   */
  void set_setter(DataBroker& db, Signal::typed_set_value_function_t<T> set_value_fn) {
    db.set_setter<T>(name(), std::move(set_value_fn));
  }

  /**
   * Return the value of a signal.
   *
   * \param db Instance of the DataBroker
   * \return Pointer to the value of the signal, nullptr if the signal does not exist
   */
  auto value(const DataBroker& db) const { return db.value<T>(name()); }
  /**
   * Set the value of a signal.
   *
   * \param db Instance of the DataBroker
   * \param value Value to be assigned to the signal
   */
  auto set_value(DataBroker& db, const T& value) { db.set_value<T>(name(), value); }
};

/**
  * SignalDescriptorImpl implements a SignalDescriptor for names
  *
  * \tparam T Type of the signal
  */
template <typename T, const char* NAME>
struct SignalDescriptorImpl : public SignalDescriptorBase<T, StaticName<NAME>> {
  using SignalDescriptorBase<T, StaticName<NAME>>::SignalDescriptorBase;
};
/**
  * SignalDescriptorImpl implements a SignalDescriptor for dynamic names
  *
  * \tparam T Type of the signal
  */
template <typename T>
struct SignalDescriptorImpl<T, nullptr> : public SignalDescriptorBase<T, DynamicName> {
  using SignalDescriptorBase<T, DynamicName>::SignalDescriptorBase;
};

/**
  * SignalDescriptor reflects properties of a signal at compile-/run-time
  *
  * \tparam T Type of the signal
  * \tparam NAME compile-time name, nullptr otherwise
  * \note: Design-Goals:
  * - Design-#1: Datatype of a signal shall be available at compile time
  * \note: Remarks:
  * - The declaration of a descriptor does not imply the availability of the coresponding signal at runtime.
  *   Likewise a C/C++ header does not imply that the coresponding symbols can be resolved at runtime.
  */
template <typename T, const char* NAME = nullptr>
struct SignalDescriptor : public SignalDescriptorImpl<T, NAME> {
  using SignalDescriptorImpl<T, NAME>::SignalDescriptorImpl;
};

template <typename T, const char* NAME>
struct SignalTemplate : public StaticName<NAME> {
 private:
  template <template <typename> class TARGET, typename... ARGS>
  static auto format(ARGS&&... args) {
    auto name = fmt::format(StaticName<NAME>::name(), std::forward<ARGS>(args)...);
    return TARGET<T>(name);
  }

 public:
  template <typename... ARGS>
  static auto specialize(ARGS&&... args) {
    return format<SignalDescriptor>(std::forward<ARGS>(args)...);
  }

  template <typename... ARGS>
  static auto partial(ARGS&&... args) {
    return format<SignalTemplate>(std::forward<ARGS>(args)...);
  }
};

// template <typename T, const char* NAME>
// struct SignalDescriptor : public SignalDescriptor<T, StaticName<NAME>> {
//using type = typename SignalDescriptorImpl<T, NAME>::type;
// };

// template <typename T>
// struct IsSignalDescriptor : std::false_type {};
// template <typename T, const char* NAME>
// struct IsSignalDescriptor<SignalDescriptor<T, NAME>> : std::true_type {};
// template <typename T>
// constexpr bool is_signal_descriptor_v() {
//   return IsSignalDescriptor<T>::value;
// }

}  // namespace databroker

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

#define CLOE_DATABROKER_TEMPLATE_INSTANTATION_TYPES()

CLOE_DATABROKER_TEMPLATE_INSTANTATION_TYPES()

// Goal g1 implies that the preprocessor macros are not undefined.

}  // namespace cloe

#endif  // CLOE_DATA_BROKER_HPP_
