#include "lua_simulation_driver.hpp"

#include "coordinator.hpp"

#include "lua_bindings.hpp"

#include "lua_action.hpp"
#include "lua_api.hpp"

#include <cloe/model.hpp>
#include <fable/conf.hpp>
#include <fable/utility/sol.hpp>

namespace engine {

LuaSimulationDriver::LuaSimulationDriver(std::unique_ptr<sol::state> lua)
    : lua_(std::move(lua)),
    data_broker_binding_(std::make_unique<cloe::databroker::LuaDataBrokerBinding>(*lua_)) {}

void LuaSimulationDriver::initialize(const SimulationSync &sync, Coordinator& scheduler, cloe::DataBroker &/*db*/) {
  auto types_tbl = sol::object(cloe::luat_cloe_engine_types(*lua_)).as<sol::table>();
  register_usertype_coordinator(types_tbl, sync);
  cloe::luat_cloe_engine_state(*lua_)["scheduler"] = std::ref(scheduler);
}

void LuaSimulationDriver::register_action_factories(cloe::Registrar& registrar) {
  registrar.register_action<actions::LuaFactory>(*lua_);
}

void LuaSimulationDriver::alias_signals(cloe::DataBroker& dataBroker) {
  bool aliasing_failure = false;
  // Read cloe.alias_signals
  sol::object signal_aliases = cloe::luat_cloe_engine_initial_input(*lua_)["signal_aliases"];
  auto type = signal_aliases.get_type();
  switch (type) {
    // cloe.alias_signals: expected is a list (i.e. table) of 2-tuple each strings
    case sol::type::table: {
      sol::table alias_signals = signal_aliases.as<sol::table>();
      auto tbl_size = std::distance(alias_signals.begin(), alias_signals.end());
      //for (auto& kv : alias_signals)
      for (int i = 0; i < tbl_size; i++) {
        //sol::object value = kv.second;
        sol::object value = alias_signals[i + 1];
        sol::type type = value.get_type();
        switch (type) {
          // cloe.alias_signals[i]: expected is a 2-tuple (i.e. table) each strings
          case sol::type::table: {
            sol::table alias_tuple = value.as<sol::table>();
            auto tbl_size = std::distance(alias_tuple.begin(), alias_tuple.end());
            if (tbl_size != 2) {
              // clang-format off
              logger()->error(
                  "One or more entries in 'cloe.alias_signals' does not consist out of a 2-tuple. "
                  "Expected are entries in this format { \"regex\" , \"short-name\" }"
                  );
              // clang-format on
              aliasing_failure = true;
              continue;
            }

            sol::object value;
            sol::type type;
            std::string old_name;
            std::string alias_name;
            value = alias_tuple[1];
            type = value.get_type();
            if (sol::type::string != type) {
              // clang-format off
              logger()->error(
                  "One or more parts in a tuple in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                  "Expected are entries in this format { \"regex\" , \"short-name\" }",
                  static_cast<int>(type));
              // clang-format on
              aliasing_failure = true;
            } else {
              old_name = value.as<std::string>();
            }

            value = alias_tuple[2];
            type = value.get_type();
            if (sol::type::string != type) {
              // clang-format off
              logger()->error(
                  "One or more parts in a tuple in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                  "Expected are entries in this format { \"regex\" , \"short-name\" }",
                  static_cast<int>(type));
              // clang-format on
              aliasing_failure = true;
            } else {
              alias_name = value.as<std::string>();
            }
            try {
              dataBroker.alias(old_name, alias_name);
              // clang-format off
              logger()->info(
                  "Aliasing signal '{}' as '{}'.",
                  old_name, alias_name);
              // clang-format on
            } catch (const std::logic_error& ex) {
              // clang-format off
              logger()->error(
                  "Aliasing signal specifier '{}' as '{}' failed with this error: {}",
                  old_name, alias_name, ex.what());
              // clang-format on
              aliasing_failure = true;
            } catch (...) {
              // clang-format off
              logger()->error(
                  "Aliasing signal specifier '{}' as '{}' failed.",
                  old_name, alias_name);
              // clang-format on
              aliasing_failure = true;
            }
          } break;
          // cloe.alias_signals[i]: is not a table
          default: {
            // clang-format off
            logger()->error(
                "One or more entries in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                "Expected are entries in this format { \"regex\" , \"short-name\" }",
                static_cast<int>(type));
            // clang-format on
            aliasing_failure = true;
          } break;
        }
      }

    } break;
    case sol::type::none:
    case sol::type::lua_nil: {
      // not defined -> nop
    } break;
    default: {
      // clang-format off
      logger()->error(
          "Expected symbol 'cloe.alias_signals' has unexpected datatype '{}'. "
          "Expected is a list of 2-tuples in this format { \"regex\" , \"short-name\" }",
          static_cast<int>(type));
      // clang-format on
      aliasing_failure = true;
    } break;
  }
  if (aliasing_failure) {
    throw cloe::ModelError("Aliasing signals failed with above error. Aborting.");
  }
}
void LuaSimulationDriver::bind_signals(cloe::DataBroker& dataBroker) {
  auto& signals = dataBroker.signals();
  bool binding_failure = false;
  // Read cloe.require_signals
  sol::object value = cloe::luat_cloe_engine_initial_input(*lua_)["signal_requires"];
  auto type = value.get_type();
  switch (type) {
    // cloe.require_signals expected is a list (i.e. table) of strings
    case sol::type::table: {
      sol::table require_signals = value.as<sol::table>();
      auto tbl_size = std::distance(require_signals.begin(), require_signals.end());

      for (int i = 0; i < tbl_size; i++) {
        sol::object value = require_signals[i + 1];

        sol::type type = value.get_type();
        if (type != sol::type::string) {
          logger()->warn(
              "One entry of cloe.require_signals has a wrong data type: '{}'. "
              "Expected is a list of strings.",
              static_cast<int>(type));
          binding_failure = true;
          continue;
        }
        std::string signal_name = value.as<std::string>();

        // virtually bind signal 'signal_name' to lua
        auto iter = dataBroker[signal_name];
        if (iter != signals.end()) {
          try {
            dataBroker.bind_signal(signal_name);
            logger()->info("Binding signal '{}' as '{}'.", signal_name, signal_name);
          } catch (const std::logic_error& ex) {
            logger()->error("Binding signal '{}' failed with error: {}", signal_name,
                            ex.what());
          }
        } else {
          logger()->warn("Requested signal '{}' does not exist in DataBroker.", signal_name);
          binding_failure = true;
        }
      }
      // actually bind all virtually bound signals to lua
      data_broker_binding_->bind("signals", cloe::luat_cloe_engine(*lua_));
    } break;
    case sol::type::none:
    case sol::type::lua_nil: {
      logger()->warn(
          "Expected symbol 'cloe.require_signals' appears to be undefined. "
          "Expected is a list of string.");
    } break;
    default: {
      logger()->error(
          "Expected symbol 'cloe.require_signals' has unexpected datatype '{}'. "
          "Expected is a list of string.",
          static_cast<int>(type));
      binding_failure = true;
    } break;
  }
  if (binding_failure) {
    throw cloe::ModelError("Binding signals to Lua failed with above error. Aborting.");
  }
}
nlohmann::json LuaSimulationDriver::produce_report() const {
  return nlohmann::json{sol::object(cloe::luat_cloe_engine_state(*lua_)["report"])};
}
cloe::TriggerPtr LuaSimulationDriver::make_trigger(TriggerFactory& factory, const sol::table& tbl) {
  sol::optional<std::string> label = tbl["label"];
  auto ep = factory.make_event(fable::Conf{nlohmann::json(tbl["event"])});
  auto ap = make_action(factory, sol::object(tbl["action"]));
  sol::optional<std::string> action_source = tbl["action_source"];
  if (!label && action_source) {
    label = *action_source;
  } else {
    label = "";
  }
  sol::optional<bool> sticky = tbl["sticky"];
  auto tp = std::make_unique<cloe::Trigger>(*label, cloe::Source::LUA, std::move(ep), std::move(ap));
  tp->set_sticky(sticky.value_or(false));
  return tp;
}
cloe::ActionPtr LuaSimulationDriver::make_action(TriggerFactory& factory, const sol::object& lua) {
  if (lua.get_type() == sol::type::function) {
    return std::make_unique<actions::LuaFunction>("luafunction", lua);
  } else {
    return factory.make_action(cloe::Conf{nlohmann::json(lua)});
  }
}
std::vector<cloe::TriggerPtr> LuaSimulationDriver::yield_pending_triggers(){
  std::vector<cloe::TriggerPtr> result;
  auto triggers = sol::object(cloe::luat_cloe_engine_initial_input(*lua_)["triggers"]);
  size_t count = 0;
  for (auto& kv : triggers.as<sol::table>()) {
    result.push_back(make_trigger(trigger_factory(), kv.second));
    count++;
  }
  cloe::luat_cloe_engine_initial_input(*lua_)["triggers_processed"] = count;
  return result;
}
cloe::databroker::DataBrokerBinding* LuaSimulationDriver::data_broker_binding() {
  return data_broker_binding_.get();
}

}
