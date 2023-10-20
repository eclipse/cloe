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
#include <typeindex>
#include <vector>

#include <fmt/format.h>

#include <sol/sol.hpp>

namespace cloe {

namespace databroker {

namespace detail {

/**
  * Detects the presence of the to_lua function (based on ADL)
  */
template <typename T, typename = void>
struct has_to_lua : std::false_type {};
/**
  * Detects the presence of the to_lua function (based on ADL)
  */
template <typename T>
struct has_to_lua<
    T, std::void_t<decltype(to_lua(std::declval<sol::state_view>(), std::declval<T*>()))>>
    : std::true_type {};
/**
  * Detects the presence of the to_lua function (based on ADL)
  */
template <typename T>
constexpr bool has_to_lua_v = has_to_lua<T>::value;

/**
  * Invokes to_lua procedure, if detecting its presence
  */
template <typename T>
void to_lua(sol::state_view lua) {
  if constexpr (has_to_lua_v<T>) {
    to_lua(lua, static_cast<T*>(nullptr));
  } else {
    // nop
  }
}

}  // namespace detail

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
  [[nodiscard]] std::size_t count() const { return eventhandlers_.size(); }

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
  static_assert(!static_cast<bool>(databroker::is_incompatible_type_v<T>),
                "Incompatible-Datatype-Error.\n"
                "\n"
                "Please find the offending LOC in above line 'require from here'.\n"
                "\n"
                "Explanation/Reasoning:\n"
                "- references & void are fundamentally incompatible");
}

using SignalPtr = std::shared_ptr<Signal>;

/**
  * Function which integrates a specific datum into the Lua-VM
  */
using lua_signal_adapter_t =
    std::function<void(const SignalPtr&, sol::state_view, std::string_view)>;

/**
  * Function which declares a specific datatype to the Lua-VM
  */
using lua_signal_declarator_t = std::function<void(sol::state_view)>;

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
    explicit access_token(int /*unused*/){};
  };

  value_type value_{};
  databroker::on_value_changed_callback_t<value_type> on_value_changed_{};
  Signal* signal_{};

 public:
  BasicContainer() = default;
  BasicContainer(Signal* signal,
                 databroker::on_value_changed_callback_t<value_type>
                     on_value_changed,
                 access_token /*unused*/)
      : value_(), on_value_changed_(std::move(on_value_changed)), signal_(signal) {
    update_accessor_functions(this);
  }
  BasicContainer(const BasicContainer&) = delete;
  BasicContainer(BasicContainer<value_type>&& source) { *this = std::move(source); }

  ~BasicContainer() { update_accessor_functions(nullptr); }
  BasicContainer& operator=(const BasicContainer&) = delete;
  BasicContainer& operator=(BasicContainer&& rhs) {
    value_ = std::move(rhs.value_);
    on_value_changed_ = std::move(rhs.on_value_changed_);
    signal_ = std::move(rhs.signal_);
    update_accessor_functions(this);

    rhs.signal_ = nullptr;

    return *this;
  }
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

  [[nodiscard]] bool has_subscriber() const;
  [[nodiscard]] std::size_t subscriber_count() const;

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

/**
 * MetaInformation collects abstract metainformation
 *
 * \note: Design-Goals:
 * - Design-#1: Key-Value (cardinality: 0-1:1). The key defines the value-type.
 * - Design-#2: Type-erasing techniques shall not eradicate type-safety nor put additional validation steps onto the users.
 * \note: Implementation-Notes:
 * - Implementation-#1:
 *   Implementations like "all-is-byte-arrays" or "JSON" were considered and disregarded.
 *   E.g. JSON is a serialization format. Using JSON-Schema Design-#2 would be covered.
 *   This would imply a) a dependency on multiple levels & b) significant runtime efforts.
 *   Shooting sparrows with canons. Pure C++ can do the job in <50 LOC + some for porcellain.
 */
class MetaInformation {
 private:
  using metainformation_map_t = std::unordered_map<std::type_index, std::any>;
  metainformation_map_t metainformations_;

 public:
  /**
    * Tag which identifies the metainformation and carries the type information of the actual metainformation
    */
  template <typename T>
  struct Tag {
   public:
    using tag_type = T;
  };

 public:
  MetaInformation() = default;
  virtual ~MetaInformation() = default;

 private:
  template <typename T>
  constexpr void assert_static_type() {
    // prevent usage of references
    static_assert(std::is_reference_v<typename T::tag_type> == false,
                  "References are unsupported.");
  }

 public:
  template <typename T>
  /**
    * Removes an metainformation
    * \tparam Tag of the metainformation to be removed
    */
  void remove() {
    auto tindex = std::type_index(typeid(T));
    auto iter = metainformations_.find(tindex);
    if (iter != metainformations_.end()) {
      metainformations_.erase(iter);
    }
  }
  /**
    * Adds a metainformation
    * \tparam T Type of the metainformation-tag
    * \param metainformation_any Actual metainformation to be added
    */
  template <typename T>
  void add_any(std::any metainformation_any) {
    auto tindex = std::type_index(typeid(T));
    auto iter = metainformations_.find(tindex);
    if (iter != metainformations_.end()) {
    }
    metainformations_[tindex] = std::move(metainformation_any);
  }
  /**
     * Returns a metainformation
     * \tparam T Type of the metainformation-tag
     * \returns std:any* if the metainformation is present, nullptr otherwise
     */
  template <typename T>
  const std::any* get_any() const {
    auto tindex = std::type_index(typeid(T));
    auto iter = metainformations_.find(tindex);
    if (iter != metainformations_.end()) {
      const std::any& metainformation_any = iter->second;
      return &metainformation_any;
    } else {
      return nullptr;
    }
  }
  /**
    * Returns a metainformation
    * \tparam T Type of the metainformation-tag
    * \returns Annotation of type T::tag_type* if the metainformation is present, nullptr otherwise
    */
  template <typename T>
  std::enable_if_t<!std::is_void_v<typename T::tag_type>, const typename T::tag_type>* get() const {
    const std::any* metainformation_any = get_any<T>();
    return (metainformation_any != nullptr)
               ? std::any_cast<typename T::tag_type>(metainformation_any)
               : nullptr;
  }
  /**
    * Returns a metainformation
    * \tparam T Type of the metainformation-tag
    * \returns true if the metainformation is present, false otherwise
    */
  template <typename T>
  std::enable_if_t<std::is_void_v<typename T::tag_type>, bool> get() const {
    const std::any* metainformation_any = get_any<T>();
    return (metainformation_any != nullptr);
  }

  /**
     * Adds a metainformation
     * \tparam T Type of the metainformation-tag
     * \param metainformation Actual metainformation to be added
     * \note This overload is enabled only when the effective tag_type is move constructible
     */
  template <typename T>
  // clang-format off
  std::enable_if_t<
       !std::is_void_v<typename T::tag_type>
    && !std::is_base_of_v< Tag< T >, T >
    &&  std::is_move_constructible_v<typename T::tag_type>
  >
  // clang-format on
  add(typename T::tag_type metainformation) {
    assert_static_type<T>();

    std::any metainformation_any = std::move(metainformation);
    add_any<T>(std::move(metainformation_any));
  }
  /**
     * Adds a metainformation
     * \tparam T Type of the metainformation-tag
     * \param metainformation Actual metainformation to be added
     * \note This overload is enabled only when the effective tag_type is copy constructible
     */
  template <typename T>
  // clang-format off
  std::enable_if_t<
       !std::is_void_v<typename T::tag_type>
    && !std::is_base_of_v< Tag< T >, T >
    &&  std::is_copy_constructible_v<typename T::tag_type>
    && !std::is_move_constructible_v<typename T::tag_type>
  >
  // clang-format on
  add(const typename T::tag_type& metainformation) {
    assert_static_type<T>();

    std::any metainformation_any = metainformation;
    add_any<T>(std::move(metainformation_any));
  }
  /**
     * Adds a metainformation
     * \tparam T Type of the metainformation-tag
     * \note This overload is enabled only when the effective tag_type is void
     */
  template <typename T>
  // clang-format off
  std::enable_if_t<
    std::is_void_v<typename T::tag_type>
  >
  // clang-format on
  add() {
    std::any metainformation_any;
    add_any<T>(std::move(metainformation_any));
  }
  /**
    * Adds a metainformation
    * \tparam T Type of the metainformation-tag
    * \param metainformation Actual metainformation to be added
    * \note This overload is enabled only when the tag inherits Tag<> and is the effective tag_type
    *
    * Usage Note:
    *
    * ```
    * struct my_tag : cloe::MetaInformation::Tag<my_tag> {
    *  ...
    * };
    * ...
    * my_tag tag;
    * ...
    * add(std::move(tag));
    * ```
    */
  template <typename T>
  // clang-format off
  std::enable_if_t<
       std::is_base_of_v< Tag< T >, T >
    && std::is_same_v<T, typename T::tag_type>
  >
  // clang-format on
  add(T metainformation) {
    assert_static_type<T>();

    std::any metainformation_any = std::move(metainformation);
    add_any<T>(std::move(metainformation_any));
  }
  /**
    * Adds a metainformation constructed from the supplied parameters
    * \tparam T Type of the metainformation-tag
    * \tparam TArgs... Type of the metainformation c'tor arguments
    * \param args Arguments for the c'tor of the metainformation
    * \note This overload is enabled only when the tag inherits Tag<> and is the effective tag_type
    *
    * Usage Note:
    *
    * ```
    * add<TagType>(arg1, arg2, ...);
    *  ...
    * };
    * ```
    */
  template <typename T, typename... TArgs>
  // clang-format off
  std::enable_if_t<
       std::is_base_of_v< Tag< T >, T >
    && std::is_same_v<T, typename T::tag_type>
  >
  // clang-format on
  add(TArgs... args) {
    assert_static_type<T>();
    T metainformation(std::forward<TArgs>(args)...);
    std::any metainformation_any = std::move(metainformation);
    add_any<T>(std::move(metainformation_any));
  }
};

struct SignalDocumentation : MetaInformation::Tag<SignalDocumentation> {
  /**
    * Documentation text
    * \note Use <br> to achieve a linebreak
    */
  std::string text;

  SignalDocumentation(std::string text_) : text{std::move(text_)} {}

  friend const std::string& to_string(const SignalDocumentation& doc) { return doc.text; }
};

/**
  * Signal-Metainformation for generation of Lua documentation
  */
struct LuaAutocompletionTag : MetaInformation::Tag<LuaAutocompletionTag> {
/**
 * X-Macro: enum definition & enum-to-string conversion
 */
#define LUADATATYPE_LIST \
  X(Class, 0)            \
  X(Number, 1)           \
  X(String, 2)

  enum class LuaDatatype {
#define X(name, value) name = value,
    LUADATATYPE_LIST
#undef X
  };

  friend std::string to_string(LuaDatatype type) {
    switch (type) {
#define X(name, value)    \
  case LuaDatatype::name: \
    return #name;
      LUADATATYPE_LIST
#undef X
      default:
        return {};
    }
  }
#undef LUADATATYPE_LIST

/**
 * X-Macro: enum definition & enum-to-string conversion
 */
#define PHYSICALQUANTITIES_LIST \
  X(Dimensionless, "[]")        \
  X(Radian, "[rad]")            \
  X(Length, "[m]")              \
  X(Time, "[s]")                \
  X(Mass, "[kg]")               \
  X(Temperature, "[K]")         \
  X(ElectricCurrent, "[A]")     \
  X(Velocity, "[m/s]")          \
  X(Acceleration, "[m/s^2]")    \
  X(Jerk, "[m/s^3]")            \
  X(Jounce, "[m/s^4]")          \
  X(Crackle, "[m/s^5]")

  enum class PhysicalQuantity {
#define X(name, value) name,
    PHYSICALQUANTITIES_LIST
#undef X
  };

  friend std::string to_string(PhysicalQuantity type) {
    switch (type) {
#define X(name, value)         \
  case PhysicalQuantity::name: \
    return #value;
      PHYSICALQUANTITIES_LIST
#undef X
      default:
        return {};
    }
  }
#undef PHYSICALQUANTITIES_LIST

  /**
    * Lua datatype of the signal
    */
  LuaDatatype datatype;
  /**
    * Lua datatype of the signal
    */
  PhysicalQuantity unit;
  /**
    * Documentation text
    * \note Use <br> to achieve a linebreak
    */
  std::string text;

  LuaAutocompletionTag(LuaDatatype datatype_, PhysicalQuantity unit_, std::string text_)
      : datatype{std::move(datatype_)}, unit{std::move(unit_)}, text{std::move(text_)} {}
};

/**
 * Signal represents the properties of a signal at runtime
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
    explicit access_token(int /*unused*/){};
  };

 public:
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

 private:
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
  /// metadata accompanying the signal
  MetaInformation metainformations_;

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
  explicit Signal(access_token /*unused*/) : Signal() {}
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

  /**
   * Tags a signal with metadata
   *
   * \tparam T Type of the tag
   * \param metadata Metadata used to tag the signal
   * \note This is the overload for non-void tags (T2 != void)
   */
  template <typename T>
  std::enable_if_t<!std::is_void_v<typename T::tag_type>> add(typename T::tag_type metadata) {
    static_assert(std::is_reference_v<typename T::tag_type> == false);
    metainformations_.add<T>(metadata);
  }
  /**
   * Tags a signal with metadata constructed from parameters
   *
   * \tparam T Type of the tag
   * \tparam TArgs Type of the metadata c'tor parameters
   * \param args Metadata c'tor arguments
   * \note This is the overload for non-void tags (T2 != void)
   */
  template <typename T, typename... TArgs>
  std::enable_if_t<!std::is_void_v<typename T::tag_type>> add(TArgs&&... args) {
    static_assert(std::is_reference_v<typename T::tag_type> == false);
    metainformations_.add<T>(std::forward<TArgs>(args)...);
  }

  /**
   * Tags a signal with a void tag
   *
   * \tparam T Type of the tag
   */
  template <typename T>
  // clang-format off
  std::enable_if_t<
    std::is_void_v<typename T::tag_type>
  >
  // clang-format on
  add() {
    metainformations_.add<T>();
  }
  /**
   * Get a tag of the signal
   *
   * \tparam T Type of the tag
   * \returns const typename T::tag_type* pointing to the tag-value (or nullptr), if typename T::tag_type != void
   * \returns bool expressing the presence of the tag, if typename T::tag_type == void
   */
  template <typename T>
  auto metadata() -> decltype(metainformations_.get<T>()) {
    return metainformations_.get<T>();
  }
  /**
   * Get all tags of the signal
   */
  const MetaInformation& metadatas() const { return metainformations_; }

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
    if (names_.empty()) {
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
    if (names_.empty()) {
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
  if (signal_ != nullptr) {
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
  return signal_ != nullptr && signal_->has_subscriber();
}

template <typename T>
std::size_t BasicContainer<T>::subscriber_count() const {
  return signal_ != nullptr ? signal_->subscriber_count() : 0;
}

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
  std::unordered_map<std::type_index, lua_signal_adapter_t> bindings_{};
  std::unordered_map<std::type_index, bool> lua_declared_types_{};

 public:
  DataBroker() = default;
  explicit DataBroker(const sol::state_view& lua) : lua_(lua), signals_object_(*lua_) {}
  DataBroker(const DataBroker&) = delete;
  DataBroker(DataBroker&&) = delete;
  ~DataBroker() = default;
  DataBroker& operator=(const DataBroker&) = delete;
  DataBroker& operator=(DataBroker&&) = delete;

 private:
  /**
    * Dynamic class which embedds all signals in shape of properties into the Lua-VM
    */
  class SignalsObject {
   private:
    /**
      * Lua-Getter Function (C++ -> Lua)
      */
    using lua_getter_fn = std::function<sol::object(sol::this_state&)>;
    /**
      * Lua-Setter Function (Lua -> C++)
      */
    using lua_setter_fn = std::function<void(sol::stack_object&)>;
    /**
      * Lua accessors (getter/setter)
      */
    struct lua_accessor {
      lua_getter_fn getter;
      lua_setter_fn setter;
    };
    /**
      * Signals map (name -> accessors)
      */
    using accessors = std::unordered_map<std::string, lua_accessor>;
    /**
      * Mapped signals
      */
    accessors accessors_;
    /**
      * Lua usertype, declares this class towards Lua
      */
    sol::usertype<SignalsObject> signals_table_;

   public:
    SignalsObject(sol::state_view& lua)
        : accessors_()
        , signals_table_(lua.new_usertype<SignalsObject>(
              "SignalsObject", sol::meta_function::new_index, &SignalsObject::set_property_lua,
              sol::meta_function::index, &SignalsObject::get_property_lua)) {}

    /**
      * \brief Getter function for dynamic Lua properties
      * \param name Accessed name on Lua level
      * \param s Current Lua-state
      */
    sol::object get_property_lua(const char* name, sol::this_state s) {
      auto iter = accessors_.find(name);
      if (iter != accessors_.end()) {
        auto result = iter->second.getter(s);
        return result;
      } else {
        throw std::out_of_range(
            fmt::format("Failure to access signal '{}' from Lua since it is not bound.", name));
      }
    }
    /**
      * \brief Setter function for dynamic Lua properties
      * \param name Accessed name on Lua level
      * \param object Lua-Object assigned to the property
      */
    void set_property_lua(const char* name, sol::stack_object object) {
      auto iter = accessors_.find(name);
      if (iter != accessors_.end()) {
        iter->second.setter(object);
      } else {
        throw std::out_of_range(
            fmt::format("Failure to access signal '{}' from Lua since it is not bound.", name));
      }
    }
    /**
     * Factory which produces the gluecode to r/w Lua properties
     */
    template <typename T>
    struct LuaAccessorFactory {
      using type = T;
      using value_type = T;
      static lua_accessor make(const SignalPtr& signal) {
        lua_accessor result;
        result.getter = [signal](sol::this_state& state) -> sol::object {
          const value_type& value = signal->value<value_type>();
          return sol::make_object(state, value);
        };
        result.setter = [signal](sol::stack_object& obj) -> void {
          T value = obj.as<value_type>();
          signal->set_value<value_type>(value);
        };
        return result;
      }
    };
    /**
     * Factory which produces the gluecode to r/w Lua properties
     * \note Specialization for std::optional
     */
    template <typename T>
    struct LuaAccessorFactory<std::optional<T>> {
      using type = std::optional<T>;
      using value_type = T;
      static lua_accessor make(const SignalPtr& signal) {
        lua_accessor result;
        result.getter = [signal](sol::this_state& state) -> sol::object {
          const type& value = signal->value<type>();
          if (value) {
            return sol::make_object(state, value.value());
          } else {
            return sol::make_object(state, sol::lua_nil);
          }
        };
        result.setter = [signal](sol::stack_object& obj) -> void {
          type value;
          if (obj != sol::lua_nil) {
            value = obj.as<value_type>();
          }
          signal->set_value<type>(value);
        };
        return result;
      }
    };

    /**
      *  \brief Binds one signal to Lua
      *  \param signal signal to be bound to Lua
      *  \param lua_name name of the signal in Lua
      */
    template <typename T>
    void bind(const SignalPtr& signal, std::string_view lua_name) {
      lua_accessor accessor = LuaAccessorFactory<T>::make(signal);
      auto inserted = accessors_.try_emplace(std::string(lua_name), std::move(accessor));
      if (!inserted.second) {
        throw std::out_of_range(fmt::format(
            "Failure adding lua-accessor for signal {}. Name already exists.", lua_name));
      }
    }
  };
  /**
    * state_view of Lua
    */
  std::optional<sol::state_view> lua_{};
  /**
    * Instance of signals body which incorporates all bound signals
    */
  std::optional<SignalsObject> signals_object_{};

 public:
  /**
    * \brief Declares a DataType to Lua (if not yet done)
    * \note: The function can be used independent of a bound Lua instance
    */
  template <typename T>
  void declare_type(lua_signal_declarator_t type_declarator) {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    if (lua_.has_value()) {
      std::type_index type{typeid(compatible_type)};
      auto iter = lua_declared_types_.find(type);
      if (iter == lua_declared_types_.end()) {
        lua_declared_types_[type] = true;
        // declare type
        type_declarator(*lua_);
      }
    }
  }

 private:
  /**
    * \brief Declares a DataType to Lua (if not yet done)
    * \note: The function can be used independent of a bound Lua instance
    */
  template <typename T>
  void declare() {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;
    if (lua_.has_value()) {
      // Check whether this type was already processed, if not declare it and store an adapter function in bindings_
      std::type_index type{typeid(compatible_type)};
      auto iter = bindings_.find(type);
      if (iter == bindings_.end()) {
        // Check wether this type was already declared to the Lua-VM, if not declare it
        auto declared_types_iter = lua_declared_types_.find(type);
        if (declared_types_iter == lua_declared_types_.end()) {
          lua_declared_types_[type] = true;
          ::cloe::databroker::detail::to_lua<T>(*lua_);
        }

        // Create adapter for Lua-VM
        lua_signal_adapter_t adapter = [this](const SignalPtr& signal, sol::state_view state,
                                              std::string_view lua_name) {
          //adapter_impl<T>(signal, state, lua_name);
          // Subscribe to the value-changed event to indicate the signal is used
          signal->subscribe<T>([](const T&) {});
          // Implement the signal as a property in Lua
          signals_object_->bind<T>(signal, lua_name);
        };
        // Store adapter function
        bindings_.emplace(type, std::move(adapter));
      }
    }
  }

 public:
  /**
    * \brief Binds a signal to the Lua-VM
    * \param signal_name Name of the signal
    * \param lua_name Name of the table/variable used in Lua
    * \note The bind-method needs to be invoked at least once (in total) to bring all signal bindings into effect
    */
  void bind_signal(std::string_view signal_name, std::string_view lua_name) {
    if (!lua_.has_value()) {
      throw std::logic_error(
          "DataBroker: Binding a signal to Lua must not happen, before binding the Lua "
          "context.");
    }

    SignalPtr signal = this->signal(signal_name);
    auto type = std::type_index(*signal->type());

    auto iter = bindings_.find(type);
    if (iter == bindings_.end()) {
      throw std::runtime_error(
          "DataBroker: <internal logic error>: Lua type binding not implemented");
    }
    const lua_signal_adapter_t& adapter = iter->second;
    adapter(signal, (*lua_), lua_name);
  }

  /**
   * \brief Binds a signal to the Lua-VM
   * \param signal_name Name of the signal
   * \note The bind-method needs to be invoked at least once (in total) to bring all signal bindings into effect
   */
  void bind_signal(std::string_view signal_name) { bind_signal(signal_name, signal_name); }

  /**
   * \brief Binds the signals-object to Lua
   * \param signals_name Name which shall be used for the table
   * \param parent_table Parent-table to use
   */
  void bind(std::string_view signals_name, sol::table parent) {
    parent[signals_name] = &(*signals_object_);
  }

  void bind(std::string_view signals_name) { (*lua_)[signals_name] = &(*signals_object_); }

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

    declare<compatible_type>();

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

    declare<compatible_type>();

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

/**
 * Tags a signal
 */
template <typename T>
struct SignalDescriptorBase {
  using type = T;
};
template <typename T, const char*...>
struct SignalDescriptorImpl : public SignalDescriptorBase<T> {
  using type = typename SignalDescriptorBase<T>::type;

  template <typename TFirstName, typename... TNames>
  static auto implement(DataBroker& db, TFirstName firstName, TNames... names) {
    auto signal = TypedSignal<T>(db.implement<type>(firstName));
    ((db.alias(signal, names)), ...);
    return signal;
  }
  template <typename TFirstName, typename... TNames>
  static auto declare(DataBroker& db, TFirstName firstName, TNames... names) {
    auto signal = TypedSignal<T>(db.declare<type>(firstName));
    ((db.alias(signal, names)), ...);
    return signal;
  }
};

template <typename T, const char* FIRSTNAME, const char*... NAMES>
struct SignalDescriptorImpl<T, FIRSTNAME, NAMES...> : public SignalDescriptorBase<T> {
  using type = typename SignalDescriptorBase<T>::type;

  static constexpr const char* Name() { return FIRSTNAME; }

  /**
   * Implements the signal
   *
   * \param db Instance of the DataBroker
   * \return Container<type>, the container of the signal
   */
  static auto implement(DataBroker& db) {
    auto container = db.implement<type>(Name());
    auto signal = db.signal(Name());
    ((db.alias(signal, NAMES)), ...);
    return container;
  }
  /**
   * Declares the signal
   *
   * \param db Instance of the DataBroker
   * \return TypeSignal<type>, the signal
   */
  static auto declare(DataBroker& db) {
    auto signal = TypedSignal<T>(db.declare<type>(Name()));
    ((db.alias(signal, NAMES)), ...);
    return TypedSignal<type>(std::move(signal));
  }

  /**
   * Return the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return TypeSignal<type>, the signal
   */
  static auto signal(DataBroker& db) {
    auto signal = db.signal(Name());
    return TypedSignal<type>(std::move(signal));
  }

  /**
   * Return the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return const Signal::typed_get_value_function_t<type>&, getter-function of the signal
   */
  static auto getter(DataBroker& db) { return db.getter<type>(Name()); }
  /**
   * Sets the getter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \param get_value_fn getter-function of the signal
   */
  static void set_getter(DataBroker& db, Signal::typed_get_value_function_t<type> get_value_fn) {
    db.set_getter<type>(Name(), std::move(get_value_fn));
  }
  /**
   * Return the setter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \return const Signal::typed_set_value_function_t<type>&, setter-function of the signal
   */
  static auto setter(DataBroker& db) { return db.setter<type>(Name()); }
  /**
   * Sets the setter-function of a signal.
   *
   * \param db Instance of the DataBroker
   * \param set_value_fn setter-function of the signal
   */
  static void set_setter(DataBroker& db, Signal::typed_set_value_function_t<type> set_value_fn) {
    db.set_setter<type>(Name(), std::move(set_value_fn));
  }

  /**
   * Return the value of a signal.
   *
   * \param db Instance of the DataBroker
   * \return Pointer to the value of the signal, nullptr if the signal does not exist
   */
  static auto value(DataBroker& db) { return db.value<type>(Name()); }
  /**
   * Set the value of a signal.
   *
   * \param db Instance of the DataBroker
   * \param value Value to be assigned to the signal
   */
  static auto set_value(DataBroker& db, const T& value) { db.set_value<type>(value); }
};

/**
  * SignalDescriptor reflects properties of a signal at compile-time
  *
  * \note: Design-Goals:
  * - Design-#1: Datatype of a signal shall be available at compile time
  * \note: Remarks:
  * - The declaration of a descriptor does not imply the availability of the coresponding signal at runtime.
  *   Likewise a C/C++ header does not imply that the coresponding symbols can be resolved at runtime.
  */
template <typename T, const char*... NAMES>
struct SignalDescriptor : public SignalDescriptorImpl<T, NAMES...> {
  using type = typename SignalDescriptorImpl<T, NAMES...>::type;
};

template <typename T>
struct IsSignalDescriptor : std::false_type {};
template <typename T, const char*... NAMES>
struct IsSignalDescriptor<SignalDescriptor<T, NAMES...>> : std::true_type {};
template <typename T>
constexpr bool is_signal_descriptor_v() {
  return IsSignalDescriptor<T>::value;
}

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
