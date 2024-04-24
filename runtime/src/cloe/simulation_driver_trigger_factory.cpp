#include <cloe/simulation_driver_trigger_factory.hpp>

namespace cloe {

namespace {
template <typename T, typename M>
std::unique_ptr<T> make_some(const fable::Conf& c, const M& m) {
  bool alternate = (*c).type() == nlohmann::json::value_t::string;

  auto get_factory = [&](const std::string& key) -> const auto& {
    if (m.count(key) == 0) {
      if (std::is_same<T, cloe::Action>::value) {
        throw TriggerUnknownAction(key, c);
      }
      if (std::is_same<T, cloe::Action>::value) {
        throw TriggerUnknownEvent(key, c);
      }
    } else {
      return m.at(key);
    }
    throw std::runtime_error("Unknown production type for make_some factory!");
  };

  if (alternate) {
    // This section tries to create a new action/event by using the alternate
    // string description. Not every event/action supports this though!
    auto input = c.get<std::string>();

    std::string name;
    std::string argument;
    auto found = input.find("=");
    if (found == std::string::npos) {
      name = input;
    } else {
      name = input.substr(0, found);
      argument = input.substr(found + 1);
    }

    const auto& factory = get_factory(name);
    try {
      return factory->make(argument);
    } catch (cloe::TriggerError& e) {
      c.throw_error(e.what());
    }
  } else {
    auto name = c.get<std::string>("name");
    const auto& factory = get_factory(name);
    return factory->make(c);
  }
}

}  // anonymous namespace

void DriverTriggerFactory::register_action(const std::string& key, cloe::ActionFactoryPtr&& af) {
  if (actions_.count(key) != 0) {
    throw cloe::Error("duplicate action name not allowed");
  }
  logger()->debug("Register action: {}", key);
  af->set_name(key);
  actions_[key] = std::move(af);
}
void DriverTriggerFactory::register_event(const std::string& key, cloe::EventFactoryPtr&& ef) {
  if (events_.count(key) != 0) {
    throw cloe::Error("duplicate event name not allowed");
  }
  logger()->debug("Register event: {}", key);
  ef->set_name(key);
  events_[key] = std::move(ef);
}
const DriverTriggerFactory::ActionFactoryMap& DriverTriggerFactory::actions() const { return actions_; }
const DriverTriggerFactory::EventFactoryMap& DriverTriggerFactory::events() const { return events_; }
cloe::ActionPtr DriverTriggerFactory::make_action(const cloe::Conf& c) const {
  return make_some<cloe::Action>(c, actions_);
}
cloe::EventPtr DriverTriggerFactory::make_event(const cloe::Conf& c) const {
  return make_some<cloe::Event>(c, events_);
}
cloe::TriggerPtr DriverTriggerFactory::make_trigger(cloe::Source s, const cloe::Conf& c) const {
  cloe::EventPtr ep;
  cloe::ActionPtr ap;
  bool opt = c.get_or("optional", false);
  try {
    ep = make_event(c.at("event"));
    ap = make_action(c.at("action"));
  } catch (cloe::TriggerError& e) {
    if (opt) {
      logger()->warn("Ignoring optional trigger ({}): {}", e.what(), c->dump());
      return nullptr;
    } else {
      throw;
    }
  }
  auto label = c.get_or<std::string>("label", "");
  auto t = std::make_unique<cloe::Trigger>(label, s, std::move(ep), std::move(ap));
  t->set_sticky(c.get_or("sticky", false));
  t->set_conceal(c.get_or("conceal", false));
  return t;
}
}
