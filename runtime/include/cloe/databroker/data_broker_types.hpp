#pragma once

#include <functional>

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

}

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
}
