#pragma once

#include "data_broker_signal.hpp"

namespace cloe {

template <typename T>
inline void BasicContainer<T>::update_accessor_functions(BasicContainer* container) {
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
inline bool BasicContainer<T>::has_subscriber() const {
  return signal_ != nullptr && signal_->has_subscriber();
}

template <typename T>
inline std::size_t BasicContainer<T>::subscriber_count() const {
  return signal_ != nullptr ? signal_->subscriber_count() : 0;
}

}
