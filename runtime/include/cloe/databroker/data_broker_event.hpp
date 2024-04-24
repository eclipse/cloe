#pragma once

#include <functional>
#include <vector>
#include <utility>

namespace cloe::databroker {


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

}
