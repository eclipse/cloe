#pragma once

#include "trigger.hpp"

#include <map>
#include <string>
namespace cloe {

/**
 * TriggerUnknownAction is thrown when an Action cannot be created because the
 * ActionFactory cannot be found.
 */
class TriggerUnknownAction : public cloe::TriggerInvalid {
 public:
  TriggerUnknownAction(const std::string& key, const cloe::Conf& c)
      : TriggerInvalid(c, "unknown action: " + key), key_(key) {}
  virtual ~TriggerUnknownAction() noexcept = default;

  /**
   * Return key that is unknown.
   */
  const char* key() const { return key_.c_str(); }

 private:
  std::string key_;
};

/**
 * TriggerUnknownEvent is thrown when an Event cannot be created because the
 * EventFactory cannot be found.
 */
class TriggerUnknownEvent : public cloe::TriggerInvalid {
 public:
  TriggerUnknownEvent(const std::string& key, const cloe::Conf& c)
      : TriggerInvalid(c, "unknown event: " + key), key_(key) {}
  virtual ~TriggerUnknownEvent() noexcept = default;

  /**
   * Return key that is unknown.
   */
  const char* key() const { return key_.c_str(); }

 private:
  std::string key_;
};

class DriverTriggerFactory {
 public:
  using ActionFactoryMap = std::map<std::string, cloe::ActionFactoryPtr>;
  using EventFactoryMap = std::map<std::string, cloe::EventFactoryPtr>;

  void register_action(const std::string& key, cloe::ActionFactoryPtr&& af);
  void register_event(const std::string& key, cloe::EventFactoryPtr&& ef);

  [[nodiscard]] cloe::ActionPtr make_action(const cloe::Conf& c) const;
  [[nodiscard]] cloe::EventPtr make_event(const cloe::Conf& c) const;
  [[nodiscard]] cloe::TriggerPtr make_trigger(cloe::Source s, const cloe::Conf& c) const;

  [[nodiscard]] const ActionFactoryMap &actions() const;
  [[nodiscard]] const EventFactoryMap &events() const;

  [[nodiscard]] static cloe::Logger logger() { return cloe::logger::get("cloe"); }

 private:
  ActionFactoryMap actions_;
  EventFactoryMap events_;
};
}
