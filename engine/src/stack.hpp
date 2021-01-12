/*
 * Copyright 2020 Robert Bosch GmbH
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
 * \file stack.hpp
 * \see  stack.cpp
 * \see  stack_test.cpp
 */

#pragma once

#include <map>      // for map<>
#include <memory>   // for shared_ptr<>
#include <set>      // for set<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <boost/filesystem/path.hpp>  // for path
#include <boost/optional.hpp>         // for optional<>

#include <cloe/component.hpp>        // for ComponentFactory
#include <cloe/controller.hpp>       // for ControllerFactory
#include <cloe/core.hpp>             // for Conf, Confable, Json
#include <cloe/simulator.hpp>        // for SimulatorFactory
#include <cloe/trigger.hpp>          // for Source
#include <cloe/utility/command.hpp>  // for Command

#include "plugin.hpp"  // for Plugin

#ifndef CLOE_STACK_VERSION
#define CLOE_STACK_VERSION "4"
#endif

#ifndef CLOE_XDG_SUFFIX
#define CLOE_XDG_SUFFIX "cloe"
#endif

#ifndef CLOE_CONFIG_HOME
#define CLOE_CONFIG_HOME "${XDG_CONFIG_HOME-${HOME}/.config}/" CLOE_XDG_SUFFIX
#endif

#ifndef CLOE_DATA_HOME
#define CLOE_DATA_HOME "${XDG_DATA_HOME-${HOME}/.local/share}/" CLOE_XDG_SUFFIX
#endif

#define CLOE_SIMULATION_UUID_VAR "CLOE_SIMULATION_UUID"

namespace cloe {

/**
 * PersistentConfable holds on to the last Conf that was used on it,
 * so that in the case of later problems we have a handle on the Conf
 * responsible. This doesn't work well if multiple Confs are applied
 * before evaluation.
 */
class PersistentConfable : public Confable {
 public:
  const Conf& conf() const { return conf_; }

  void from_conf(const Conf& c) {
    Confable::from_conf(c);
    conf_ = c;
  }

 protected:
  Conf conf_;
};

inline auto id_prototype() { return schema::make_prototype<std::string>().c_identifier(); }
inline auto id_path_prototype() {
  return schema::make_prototype<std::string>().pattern("^([a-zA-Z_][a-zA-Z0-9_]*/?)+$");
}

// --------------------------------------------------------------------------------------------- //

/**
 * IncludeConf is a relative or absolute filepath that should be included in
 * the stack configuration.
 */
using IncludeConf = boost::filesystem::path;
using IncludeSchema = decltype(schema::make_schema(static_cast<IncludeConf*>(nullptr), ""));
using IncludesSchema = schema::Array<IncludeConf, IncludeSchema>;

// --------------------------------------------------------------------------------------------- //

/**
 * LoggingConf describes a change to the logging system.
 *
 * ```json
 * "logging": [
 *  {
 *    "name": "*",
 *    "pattern": "%L%L %H:%M:%S.%e [%n] %v",
 *    "level": "debug"
 *  },
 *  {
 *    "level": "debug",
 *    "name": "cloe/simulation"
 *  },
 *  {
 *    "level": "trace",
 *    "name": "cloe/json"
 *  }
 * ]
 * ```
 *
 * Given a JSON section, this function takes the following structure:
 *
 * ```json
 * [
 *   { name: "*",              level: "info" },
 *   { name: "cloe",           level: "debug" },
 *   { name: "cloe/webserver", level: "warn" },
 *   { name: "cloe",           pattern: "*** [%H:%M:%S %z] [thread %t] %v ***" }
 * ]
 * ```
 *
 * That is, each item that configures a logger must specify a `name` field,
 * then optionally any number of the following fields:
 *
 *  - `level` string, which sets the level of the logger
 *    - trace
 *    - debug
 *    - info
 *    - warn(ing)?
 *    - err(or)?
 *    - critical|fatal
 *    - off|disable
 *  - `pattern` string, which sets the output pattern of the logger.
 *    Please see [this documentation](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)
 *    for the syntax of patterns.
 *
 * A ConfigureException may be thrown.
 */
struct LoggingConf : public Confable {
  std::string name;
  boost::optional<std::string> pattern;
  boost::optional<LogLevel> level;

 public:  // Special
  void apply() const;

 public:  // Confable Overrides
  CONFABLE_SCHEMA(LoggingConf) {
    using namespace schema;  // NOLINT(build/namespaces)
    return Schema{
        {"name", make_schema(&name, "name of the logger to configure").require()},
        {"pattern", make_schema(&pattern, "pattern of the logger")},
        {"level", make_schema(&level, "level of the logger")},
    };
  }

  void validate(const Conf& c) const override {
    const auto& s = this->schema();
    s.validate(c);
    if (!c.has("pattern") && !c.has("level")) {
      throw SchemaError(c, s.json_schema(),
                        "require at least one of 'pattern' or 'level' properties");
    }
  }
};

// --------------------------------------------------------------------------------------------- //

struct ServerConf : public Confable {
  bool listen{true};
  std::string listen_address{"127.0.0.1"};
  uint16_t listen_port{8080};
  uint16_t listen_threads{10};
  std::string api_prefix{"/api"};
  std::string static_prefix{""};

 public:  // Confable Overrides
  CONFABLE_SCHEMA(ServerConf) {
    using namespace schema;  // NOLINT(build/namespaces)
    return Schema{
        {"listen", make_schema(&listen, "whether web server is enabled")},
        {"listen_address", make_schema(&listen_address, "address web server should listen at")},
        {"listen_port", make_schema(&listen_port, "port web server should listen at")},
        {"listen_threads", make_schema(&listen_threads, "threads web server should use")},
        {"static_prefix", make_schema(&static_prefix, "endpoint prefix for static resources")},
        {"api_prefix", make_schema(&api_prefix, "endpoint prefix for API resources")},
    };
  }
};

// --------------------------------------------------------------------------------------------- //

/**
 * PluginConf describes the configuration for loading one or more plugins from
 * path.
 */
struct PluginConf : public PersistentConfable {
  /** Filesystem path to file or directory. */
  boost::filesystem::path plugin_path{};

  /** Name to give plugin if path is to a single file. */
  boost::optional<std::string> plugin_name{};

  /** Prefix for plugin name(s). */
  boost::optional<std::string> plugin_prefix{};

  /** Do not fail if path does not exist. */
  boost::optional<bool> ignore_missing{};

  /**
   * Do not fail if path exists but plugin cannot be loaded.
   * This is especially useful if trying to load from several directories,
   * such as /usr/lib/cloe/plugins.
   */
  boost::optional<bool> ignore_failure{};

  /**
   * If a plugin with the same name exists, replace it with this one.
   *
   * This is dependent on the order of plugin loading, which is determined by
   * the order of configuration files.
   */
  boost::optional<bool> allow_clobber{};

 public:  // Constructors
  PluginConf() = default;
  explicit PluginConf(const std::string& p) : plugin_path(p) {}

 public:  // Special
  /**
   * Return canonical path to plugin.
   *
   * Paths that start with "builtin://" are returned as is.
   * Otherwise, the plugin path is resolved to an absolute path,
   * or returned in the system native format.
   */
  std::string canonical() const;

 public:  // Confable Overrides
  CONFABLE_SCHEMA(PluginConf) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    auto proto = String(nullptr, "").c_identifier();
    return Struct{
        {"path", make_schema(&plugin_path, "absolute or relative path to plugin").require().not_empty().normalize(true)},
        {"name", make_schema(&plugin_name, proto, "alternative name plugin is available by")},
        {"prefix", make_schema(&plugin_prefix, proto, "prefix the plugin name with this")},
        {"ignore_missing", make_schema(&ignore_missing, "ignore not-exist errors")},
        {"ignore_failure", make_schema(&ignore_failure, "ignore plugin loading errors")},
        {"allow_clobber", make_schema(&allow_clobber, "replace same-named plugins")},
    };
    // clang-format on
  }
};

using PluginsSchema = schema_type<std::vector<PluginConf>>::type;

// --------------------------------------------------------------------------------------------- //

/**
 * The mode that the watchdog operates in.
 *
 * If not set to `Off`, each state is launched asynchronously and the mode
 * determines what happens when the operation times out.
 */
enum class WatchdogMode {
  Off,    ///< Disable the watchdog entirely.
  Log,    ///< Log infractions but nothing else.
  Abort,  ///< Abort _after_ the state returns.
  Kill,   ///< Kill the program immediately.
};

// clang-format off
ENUM_SERIALIZATION(WatchdogMode, ({
  {WatchdogMode::Off, "off"},
  {WatchdogMode::Log, "log"},
  {WatchdogMode::Abort, "abort"},
  {WatchdogMode::Kill, "kill"},
}))
// clang-format on

struct EngineConf : public Confable {
  // Parsing:
  std::vector<std::string> ignore_sections{};

  // Security:
  bool security_enable_hooks{true};
  bool security_enable_commands{false};
  bool security_enable_includes{true};
  size_t security_max_include_depth{64};

  // Plugins:
  std::vector<std::string> plugin_path{};
  bool plugins_ignore_missing{false};
  bool plugins_ignore_failure{false};
  bool plugins_allow_clobber{true};

  // Hooks:
  std::vector<Command> hooks_pre_connect{};
  std::vector<Command> hooks_post_disconnect{};

  // Triggers:
  bool triggers_ignore_source{false};

  // Output:
  boost::optional<boost::filesystem::path> registry_path{CLOE_DATA_HOME "/registry"};
  boost::optional<boost::filesystem::path> output_path{"${CLOE_SIMULATION_UUID}"};
  boost::optional<boost::filesystem::path> output_file_config{"config.json"};
  boost::optional<boost::filesystem::path> output_file_result{"result.json"};
  boost::optional<boost::filesystem::path> output_file_triggers{"triggers.json"};
  bool output_clobber_files{true};

  /**
   * Number of milliseconds between states when waiting for continuation.
   *
   * (This occurs primarily in the PAUSE and KEEP_ALIVE simulation states.)
   */
  std::chrono::milliseconds polling_interval{100};

  /**
   * Mode the watchdog operates in.
   *
   * \see `WatchdogMode`
   */
  WatchdogMode watchdog_mode{WatchdogMode::Off};

  /**
   * Number of milliseconds to wait before activating the watchdog.
   *
   * If set to a non-positive integer, including zero, the watchdog is disabled
   * and only the states defined in `watchdog_state_timeout` use the watchdog.
   *
   * This value should be greater than `polling_interval` to prevent the
   * watchdog activating during normal operation; this is not enforced however.
   */
  std::chrono::milliseconds watchdog_default_timeout{90'000};

  /**
   * Number of milliseconds to wait per state before activating the watchdog.
   *
   * If a state is set to `null`, then the default applies. If the state is set
   * to zero or a negative number, then this state is exempt from the watchdog.
   *
   * The following states are defined (in `engine/src/simulation.cpp`):
   *
   *     CONNECT            * 5 minutes
   *     START
   *     STEP_BEGIN
   *     STEP_SIMULATORS
   *     STEP_CONTROLLERS
   *     STEP_END
   *     PAUSE
   *     RESUME
   *     SUCCESS
   *     FAIL
   *     ABORT              * 90 seconds
   *     STOP               * 5 minutes
   *     RESET
   *     KEEP_ALIVE
   *     DISCONNECT         * 10 minutes
   *
   * The states with an asterisk are given defaults that are not affected by
   * the default timeout as these typically take longer due to I/O operations.
   */
  std::map<std::string, boost::optional<std::chrono::milliseconds>> watchdog_state_timeouts{
      {"CONNECT", std::chrono::milliseconds{300'000}},
      {"ABORT", std::chrono::milliseconds{90'000}},
      {"STOP", std::chrono::milliseconds{300'000}},
      {"DISCONNECT", std::chrono::milliseconds{600'000}},
  };

  /**
   * Whether to keep the simulation alive after termination.
   *
   * This is primarily useful for interactive tools that want to keep reading
   * from the server even after the simulation has terminated.
   */
  bool keep_alive{false};

 public:  // Confable Overrides
  CONFABLE_SCHEMA(EngineConf) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    auto dir_proto = []() { return make_prototype<boost::filesystem::path>().not_file(); };
    auto file_proto = []() { return make_prototype<boost::filesystem::path>().not_dir().resolve(false); };
    return Struct{
        {"ignore", make_schema(&ignore_sections, "JSON pointers to sections that should be ignored").extend(true)},
        {"security", Struct{
           {"enable_hooks_section",      make_schema(&security_enable_hooks, "whether to enable engine hooks")},
           {"enable_command_action",   make_schema(&security_enable_commands, "whether to enable the command action")},
           {"enable_include_section",   make_schema(&security_enable_includes, "whether to allow config files to include other files")},
           {"max_include_depth", make_schema(&security_max_include_depth, "how many recursive includes are allowed")},
        }},
        {"hooks", Struct{
           {"pre_connect", make_schema(&hooks_pre_connect, "pre-connect hooks to execute").extend(true)},
           {"post_disconnect", make_schema(&hooks_post_disconnect, "post-disconnect hooks to execute").extend(true)},
        }},
        {"plugin_path", make_schema(&plugin_path, "list of directories to scan for plugins").extend(false)},
        {"plugins", Struct{
           {"ignore_missing", make_schema(&plugins_ignore_missing, "ignore not-exist errors")},
           {"ignore_failure", make_schema(&plugins_ignore_failure, "ignore plugin loading errors")},
           {"allow_clobber", make_schema(&plugins_allow_clobber, "replace same-named plugins")},
        }},
        {"registry_path", make_schema(&registry_path, dir_proto(), "cloe registry directory")},
        {"output", Struct{
           {"path", make_schema(&output_path, dir_proto().resolve(false), "directory to dump output files in, relative to registry path")},
           {"clobber", make_schema(&output_clobber_files, "whether to clobber existing files or not")},
           {"files", Struct{
              {"config", make_schema(&output_file_config, file_proto(), "file to store config in")},
              {"result", make_schema(&output_file_result, file_proto(), "file to store simulation result in")},
              {"triggers", make_schema(&output_file_triggers, file_proto(), "file to store triggers in")},
           }},
        }},
        {"triggers", Struct{
           {"ignore_source", make_schema(&triggers_ignore_source, "ignore trigger source when reading in triggers")},
        }},
        {"polling_interval", make_schema(&polling_interval, "milliseconds to sleep when polling for next state")},
        {"watchdog", Struct{
           {"mode", make_schema(&watchdog_mode, "modus operandi of watchdog [one of: off, log, abort, kill]")},
           {"default_timeout", make_schema(&watchdog_default_timeout, "default timeout if not overridden, 0 for no timeout")},
           {"state_timeouts", make_schema(&watchdog_state_timeouts, "timeout specific to a given state, 0 for no timeout").unique_properties(false)},
        }},
        {"keep_alive", make_schema(&keep_alive, "keep simulation alive after termination")},
    };
    // clang-format on
  }
};

using EngineSchema = schema_type<EngineConf>::type;

// --------------------------------------------------------------------------------------------- //

/**
 * DefaultConf contains a black-box configuration for a combination of
 * a binding and a name, both of which are optional.
 *
 * This will be applied when instantiating a plugin, and will fail then if it
 * is incorrect. The Conf is preserved, which allows for errors to be correctly
 * pin-pointed. It should be noted that not specifying name or binding will
 * cause the conf to be applied to all instantiations, which probably isn't a
 * good idea!
 */
struct DefaultConf : public Confable {
  boost::optional<std::string> name;
  boost::optional<std::string> binding;
  Conf args;

 public:  // Confable Overrides
  CONFABLE_SCHEMA(DefaultConf) {
    using namespace schema;  // NOLINT(build/namespaces)
    return Struct{
        {"binding", make_schema(&binding, "name of binding")},
        {"name", make_schema(&name, id_prototype(), "globally unique identifier for component")},
        {"args", make_schema(&args, "defaults to set for binding/name combination").require()},
    };
  }
};

// --------------------------------------------------------------------------------------------- //

/**
 * SimulatorConf contains the configuration for a specific simulator.
 */
struct SimulatorConf : public Confable {
  const std::string binding;
  boost::optional<std::string> name;
  std::shared_ptr<SimulatorFactory> factory;
  Conf args;

 public:  // Constructors
  SimulatorConf(const std::string& b, std::shared_ptr<SimulatorFactory> f)
      : binding(b), factory(std::move(f)) {}

 public:  // Confable Overrides
  CONFABLE_SCHEMA(SimulatorConf) {
    using namespace schema;  // NOLINT(build/namespaces)
    return Struct{
        {"binding", make_const_str(binding, "name of simulator binding").require()},
        {"name", make_schema(&name, id_prototype(), "identifier override for binding")},
        {"args", make_schema(&args, factory->schema(), "factory-specific arguments")},
    };
  }
};

using SimulatorSchema = fable::schema::Factory<SimulatorConf, SimulatorFactory>;

// --------------------------------------------------------------------------------------------- //

/**
 * ControllerConf contains the configuration for a specific controller.
 */
struct ControllerConf : public Confable {
  const std::string binding;
  boost::optional<std::string> name;
  std::string vehicle;
  std::shared_ptr<ControllerFactory> factory;
  Conf args;

 public:  // Constructors
  ControllerConf(const std::string& b, std::shared_ptr<ControllerFactory> f)
      : binding(b), factory(std::move(f)) {}

 public:  // Confable Overrides
  CONFABLE_SCHEMA(ControllerConf) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    return Struct{
        {"binding", make_const_str(binding, "name of controller binding").require()},
        {"name", make_schema(&name, id_prototype(), "identifier override for binding")},
        {"vehicle", make_schema(&vehicle, "vehicle controller is assigned to").c_identifier().require()},
        {"args", make_schema(&args, factory->schema(), "factory-specific arguments")},
    };
    // clang-format on
  }
};

using ControllerSchema = fable::schema::Factory<ControllerConf, ControllerFactory>;

// --------------------------------------------------------------------------------------------- //

struct FromSimulator : public Confable {
  std::string simulator;
  std::string index_str;
  size_t index_num;

 public:  // Special
  bool is_by_name() const { return !index_str.empty(); }
  bool is_by_index() const { return index_str.empty(); }
  void clear() {
    simulator.clear();
    index_str.clear();
    index_num = 0;
  }

 public:  // Confable Overrides
  CONFABLE_SCHEMA(FromSimulator) {
    // clang-format off
      using namespace schema;  // NOLINT(build/namespaces)
      return Variant{
          Struct{
              {"simulator", make_schema(&simulator, "simulator").not_empty().require()},
              {"index", make_schema(&index_num, "index of vehicle in simulator").require()},
          },
          Struct{
              {"simulator", make_schema(&simulator, "simulator").not_empty().require()},
              {"name", make_schema(&index_str, "name of vehicle in simulator").not_empty().require()},
          },
      };
    // clang-format on
  }

  void from_conf(const Conf& c) override {
    clear();  // Avoid inconsistent state
    Confable::from_conf(c);
  }

  void to_json(Json& j) const override {
    if (is_by_index()) {
      j = Json{
          {"simulator", simulator},
          {"index", index_num},
      };
    } else {
      j = Json{
          {"simulator", simulator},
          {"name", index_str},
      };
    }
  }
};

struct ComponentConf : public Confable {
  const std::string binding;
  boost::optional<std::string> name;
  boost::optional<std::string> from;
  std::shared_ptr<ComponentFactory> factory;
  Conf args;

 public:  // Constructors
  ComponentConf(const std::string& b, std::shared_ptr<ComponentFactory> f)
      : binding(b), factory(std::move(f)) {}

 public:  // Confable Overrides
  CONFABLE_SCHEMA(ComponentConf) {
    using namespace schema;  // NOLINT(build/namespaces)
    return Struct{
        {"binding", make_const_str(binding, "name of binding").require()},
        {"name", make_schema(&name, id_prototype(), "globally unique identifier for component")},
        {"from", make_schema(&from, "component input for binding")},
        {"args", make_schema(&args, factory->schema(), "factory-specific args")},
    };
  }
};

using ComponentSchema = fable::schema::Factory<ComponentConf, ComponentFactory>;

// TODO(ben): Add AliasConf as alternative to ComponentConf.

class VehicleSchema;

/**
 * VehicleConf contains the configuration for instantiating a vehicle.
 *
 * For example:
 *
 * ```json
 * [
 *   {
 *     "from": {
 *       "simulator": "default",
 *       "index": 0
 *     },
 *     "name": "default"
 *   },
 *   {
 *     "from": "default",
 *     "name": "fuzzy",
 *   }
 * ]
 * ```
 */
struct VehicleConf : public Confable {
  std::string name;
  FromSimulator from_sim;
  std::string from_veh;
  std::map<std::string, ComponentConf> components;

 private:  // Schemas
  ComponentSchema component_schema;
  friend VehicleSchema;

 public:  // Constructors
  explicit VehicleConf(const ComponentSchema& s) : component_schema(s) {}

 public:  // Special
  bool is_from_simulator() const { return from_veh.empty(); }
  bool is_from_vehicle() const { return !from_veh.empty(); }
  void clear() {
    name.clear();
    from_sim.clear();
    from_veh.clear();
  }

 public:  // Confable Overrides
  CONFABLE_SCHEMA(VehicleConf) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    return Struct{
        {"name", make_schema(&name, "globally unique identifier for vehicle").c_identifier().require()},
        {"from", Variant{
             make_schema(&from_sim, "simulator source"),
             make_schema(&from_veh, "vehicle source").c_identifier(),
        }.require()},
        {"components", make_schema(&components, component_schema, "component configuration of vehicle")},
    };
    // clang-format on
  }

  void from_conf(const Conf& c) override {
    clear();  // Avoid inconsistent state
    Confable::from_conf(c);
  }

  void to_json(Json& j) const override {
    Json from = is_from_simulator() ? Json(from_sim) : Json(from_veh);
    j = Json{
        {"name", name},
        {"from", from},
        {"components", components},
    };
  }
};

class VehicleSchema : public schema::Base<VehicleSchema> {
  using Type = VehicleConf;
  using F = ComponentFactory;

 public:  // Constructors
  explicit VehicleSchema(Type* ptr, std::string&& desc = "")
      : Base(JsonType::object, std::move(desc)), ptr_(ptr) {}

 public:  // Special
  const std::map<std::string, std::shared_ptr<F>> factories() const {
    return components_.factories();
  }
  std::shared_ptr<F> get_factory(const std::string& key) const {
    return components_.get_factory(key);
  }
  bool has_factory(const std::string& key) const { return components_.has_factory(key); }
  void add_factory(const std::string& key, std::shared_ptr<F> f) {
    components_.add_factory(key, std::move(f));
    if (ptr_ != nullptr) {
      ptr_->component_schema = components_;
    }
  }

 public:  // Overrides
  Json json_schema() const override {
    if (ptr_ != nullptr) {
      return ptr_->schema().json_schema();
    } else {
      VehicleConf v{components_};
      return v.schema().json_schema();
    }
  }

  void validate(const Conf& c) const override {
    if (ptr_ != nullptr) {
      ptr_->schema().validate(c);
    } else {
      VehicleConf v{components_};
      v.schema().validate(c);
    }
  }

  Json serialize(const Type& x) const { return x.to_json(); }

  Type deserialize(const Conf& c) const {
    VehicleConf v{components_};
    v.from_conf(c);
    return v;
  }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    ptr_->from_conf(c);
  }

  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    ptr_->to_json(j);
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:  // State
  ComponentSchema components_;
  Type* ptr_;
};

// --------------------------------------------------------------------------------------------- //

struct TriggerConf : public PersistentConfable {
  boost::optional<std::string> label{boost::none};
  Source source{Source::FILESYSTEM};
  Conf action{};
  Conf event{};
  bool sticky{false};

 public:  // Confable Overrides
  CONFABLE_SCHEMA(TriggerConf) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    auto EANDA_SCHEMA = Variant{
      String{nullptr, "inline format"}.pattern("^[a-zA-Z0-9_/]+(=.*)?$"),
      Struct{
        {"name", id_path_prototype().require()}
      }.additional_properties(true),
    };
    return Struct{
        {"label", make_schema(&label, "description of trigger")},
        {"source", make_schema(&source, "source from which trigger originates")},
        {"event", make_schema(&event, EANDA_SCHEMA, "event").require()},
        {"action", make_schema(&action, EANDA_SCHEMA, "action").require()},
        {"sticky", make_schema(&sticky, "whether trigger should be sticky")},
        {"at", Ignore("time at which trigger was executed", JsonType::string)},
        {"since", Ignore("time since which trigger was in queue", JsonType::string)},
    };
    // clang-format on
  }
};

// --------------------------------------------------------------------------------------------- //

/**
 * The SimulationConf struct contains all configuration values for the
 * simulation itself.
 */
struct SimulationConf : public Confable {
  /**
   * Optional namespace for simulation events and actions.
   */
  boost::optional<std::string> name{boost::none};

  /**
   * Nominal model time step.
   */
  Duration model_step_width = Duration{20'000'000};  // 20 ms

  /**
   * How many times we want to retry a controller before aborting.
   *
   * If this value is negative, then we retry an infinite number of times.
   */
  int64_t controller_retry_limit{1000};

  /**
   * The number of milliseconds to sleep before retrying a controller.
   */
  std::chrono::milliseconds controller_retry_sleep{1};

  /**
   * Whether to abort on controller failure.
   *
   * If this is set to false, then the controller is just removed from the set
   * of active controllers as opposed to the entire simulation shutdown.
   */
  bool abort_on_controller_failure{true};

 public:  // Confable Overrides
  CONFABLE_SCHEMA(SimulationConf) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)
    return Struct{
        {"namespace", make_schema(&name, id_prototype(), "namespace for simulation events and actions")},
        {"model_step_width", make_schema(&model_step_width, "default model time step in ns")},
        {"controller_retry_limit", make_schema(&controller_retry_limit, "times to retry controller processing before aborting")},
        {"controller_retry_sleep", make_schema(&controller_retry_sleep, "time to sleep before retrying controller process")},
        {"abort_on_controller_failure", make_schema(&abort_on_controller_failure, "abort simulation on controller failure")},
    };
    // clang-format on
  }
};

// --------------------------------------------------------------------------------------------- //

class StackIncompleteError : public Error {
 public:
  explicit StackIncompleteError(std::vector<std::string>&& missing);

  std::string all_sections_missing(const std::string& sep = ", ") const;
  const std::vector<std::string>& sections_missing() const { return sections_missing_; }

 private:
  std::vector<std::string> sections_missing_;
};

using ConfReader = std::function<Conf(const std::string&)>;

class Stack : public Confable {
 private:  // Constants (1)
  std::vector<std::string> reserved_ids_;

 public:  // Configuration (13)
  EngineConf engine;
  ServerConf server;
  std::vector<IncludeConf> include;
  std::vector<LoggingConf> logging;
  std::vector<PluginConf> plugins;
  std::vector<DefaultConf> simulator_defaults;
  std::vector<SimulatorConf> simulators;
  std::vector<DefaultConf> controller_defaults;
  std::vector<ControllerConf> controllers;
  std::vector<DefaultConf> component_defaults;
  std::vector<VehicleConf> vehicles;
  std::vector<TriggerConf> triggers;
  SimulationConf simulation;

 private:  // Schemas (3) & Prototypes (3)
  EngineSchema engine_schema;
  IncludesSchema include_schema;
  PluginsSchema plugins_schema;

  SimulatorSchema simulator_prototype;
  ControllerSchema controller_prototype;
  VehicleSchema vehicle_prototype;

 private:  // State (3)
  std::set<std::string> scanned_plugin_paths_;
  std::map<std::string, std::shared_ptr<Plugin>> all_plugins_;
  std::vector<Conf> applied_confs_;
  ConfReader conf_reader_func_;

 public:  // Constructors
  Stack();
  Stack(const Stack& other);
  Stack(Stack&& other);
  Stack& operator=(Stack other);
  ~Stack() = default;

  friend void swap(Stack& left, Stack& right);

 public:  // Special
  Logger logger() const { return logger::get("cloe"); }

  /**
   * Set the function that performs the read operation of the inclusion of
   * a configuration file.
   *
   * The main purpose of this is to allow the engine to reset the reader to
   * a custom one that performs variable interpolation if the user has it
   * enabled.
   */
  void set_conf_reader(ConfReader fn) {
    assert(fn != nullptr);
    conf_reader_func_ = fn;
  }

  /**
   * Try to load and register one or more plugins based on the PluginConf.
   */
  void apply_plugin_conf(const PluginConf& c);

  /**
   * Try to load and register a plugin based on the PluginConf.
   *
   * Note: Unless you know what you are doing, and have read the source code
   * of this function, you should probably use apply_plugin_conf().
   */
  void insert_plugin(const PluginConf& c);

  /**
   * Register a plugin with the stack.
   *
   * This method is useful when you have a plugin that is already loaded,
   * such as those that are compiled in the engine itself.
   */
  void insert_plugin(std::shared_ptr<Plugin> p, const PluginConf& c = {});

  /**
   * Return true if there is a plugin with this name.
   */
  bool has_plugin_with_name(const std::string& key) const;

  /**
   * Return if there is a plugin with this path.
   */
  bool has_plugin_with_path(const std::string& path) const;

  /**
   * Return the loaded plugin with the following name.
   *
   * - If more than one such plugin exists, `std::out_of_range` is thrown.
   */
  std::shared_ptr<Plugin> get_plugin_with_name(const std::string& key) const;

  /**
   * Return the loaded plugin located at a path in the filesystem.
   *
   * - If no such plugin exists, `std::out_of_range` is thrown.
   */
  std::shared_ptr<Plugin> get_plugin_with_path(const std::string& key) const;

  /**
   * Return the loaded plugin or load it temporarily.
   */
  std::shared_ptr<Plugin> get_plugin_or_load(const std::string& key_or_path) const;

  /**
   * Return all loaded plugins, regardless of type.
   */
  const std::map<std::string, std::shared_ptr<Plugin>>& get_all_plugins() const {
    return all_plugins_;
  }

  std::vector<DefaultConf> get_simulator_defaults(std::string binding, std::string name) const;
  std::vector<DefaultConf> get_controller_defaults(std::string binding, std::string name) const;
  std::vector<DefaultConf> get_vehicle_defaults(std::string name) const;
  std::vector<DefaultConf> get_component_defaults(std::string binding, std::string name) const;

  /**
   * Validate own configuration.
   *
   * This goes further than what a Schema can validate, since it may check for
   * consistency and correctness (but not necessarily completeness, except for
   * any references made) of the entire configuration.
   */
  void validate() const;

  /**
   * Return true if this configuration would be valid.
   */
  bool is_valid() const;

  /**
   * Check whether all identifier relationships are valid.
   *
   * There is a single namespace for all identifiers used in a stack file:
   *
   *  - simulators
   *  - vehicles
   *  - components
   *  - controllers
   *
   * This also checks (best-effort) whether references that are made are valid.
   */
  void check_consistency() const;

  /**
   * Check whether all default configurations are correct.
   */
  void check_defaults() const;

  /**
   * Return whether all required sections (simulators, vehicles, and
   * controllers) are available.
   *
   * An incomplete stack file may still be considered as valid. There are
   * surely simulations that could be made without a defined vehicle or
   * controller. Not having a simulator on the other hand means that any
   * executed simulation is fairly pointless. But at least it will be over
   * quickly in that case.
   */
  bool is_complete() const;

  /**
   * Throw a StackIncompleteError if configuration is not complete.
   */
  void check_completeness() const;

  /**
   * Initialize is only necessary if you want default plugins to be loaded
   * without reading a configuration file.
   */
  void initialize() { from_conf(Conf{Json{{"version", CLOE_STACK_VERSION}}}); }

  /**
   * Return the current active configuration as JSON.
   */
  Json active_config() const;

  /**
   * Return a list of JSON input configurations.
   */
  Json input_config() const;

 public:  // Confable Overrides
  /**
   * Validate a configuration.
   *
   * This cannot normally be done with `schema().validate(c)`, because plugins
   * needed to be loaded in order to validate sections of the schema. This
   * requires partial application of the schema.
   *
   * This should be equivalent to from_conf() followed by validate().
   */
  void validate(const Conf& c) const override;

  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override { from_conf(c, 0); }
  void reset_schema() override;

  CONFABLE_SCHEMA(Stack) {
    // clang-format off
    using namespace schema;  // NOLINT(build/namespaces)

    return Struct{
        {"version", make_const_str(CLOE_STACK_VERSION, "version of stackfile").require()},
        {"engine", engine_schema},
        {"include", include_schema},
        {"logging", make_schema(&logging, "logging configuration").extend(true)},
        {"plugins", plugins_schema},
        {"server", make_schema(&server, "server configuration")},
        {"defaults", Struct{
           {"simulators", make_schema(&simulator_defaults, "simulator default configurations").extend(true)},
           {"controllers", make_schema(&controller_defaults, "controller default configurations").extend(true)},
           {"components", make_schema(&component_defaults, "component default configurations").extend(true)},
        }},
        {"vehicles", make_schema(&vehicles, vehicle_prototype, "vehicle configuration").extend(true)},
        {"simulators", make_schema(&simulators, simulator_prototype, "simulator configuration").extend(true)},
        {"controllers", make_schema(&controllers, controller_prototype, "controller configuration").extend(true)},
        {"triggers", make_schema(&triggers, "triggers").extend(true)},
        {"simulation", make_schema(&simulation, "simulation configuration")},
    };
    // clang-format on
  }

 protected:
  /**
   * Recursively load and apply the configuration, taking maximum depth into
   * account.
   */
  void from_conf(const Conf& c, size_t depth);
};

}  // namespace cloe
