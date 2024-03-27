#pragma once

#include "data_broker_signal.hpp"
#include "data_broker_container.hpp"

namespace cloe {

template<typename T>
inline BasicContainer<T> Signal::create_container(){
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

}
