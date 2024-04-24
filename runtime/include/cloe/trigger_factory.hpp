#pragma once

#include "trigger.hpp"

#include <map>
#include <string>

namespace cloe {
class TriggerFactory {
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

