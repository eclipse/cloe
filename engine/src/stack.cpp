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
 * \file stack.cpp
 * \see  stack.hpp
 * \see  stack_test.cpp
 */

#include "stack.hpp"

#include <algorithm>  // for transform, swap
#include <map>        // for map<>
#include <memory>     // for shared_ptr<>
#include <set>        // for set<>
#include <string>     // for string

#include <boost/algorithm/string/predicate.hpp>  // for starts_with
#include <boost/filesystem.hpp>                  // for path
namespace fs = boost::filesystem;

#include <cloe/utility/std_extensions.hpp>  // for join_vector

namespace cloe {

StackIncompleteError::StackIncompleteError(std::vector<std::string>&& missing)
    : Error("stack is incomplete, missing sections: " + utility::join_vector(missing, ", "))
    , sections_missing_(std::move(missing)) {
  this->set_explanation(R"(
  It looks like you are trying to run a stack file that is not complete,
  i.e. there are missing sections that are required.

  For a simulation to run we require three sections to be complete:

    a) simulators   (no requirements)
    b) vehicles     (requires that a simulator has been defined)
    c) controllers  (requires that a vehicle has been defined)

  These sections don't have to all be in a single stack file, but the
  final, merged stack file should contain an entry in each section.
  )");
}

std::string StackIncompleteError::all_sections_missing(const std::string& sep) const {
  return utility::join_vector(sections_missing_, sep);
}

// --------------------------------------------------------------------------------------------- //

void LoggingConf::apply() const {
  if (name == "*") {
    // Apply settings globally
    if (pattern) {
      spdlog::set_pattern(*pattern);
    }
    if (level) {
      spdlog::set_level(*level);
    }
  } else {
    // Apply settings to specific logger
    auto log = logger::get(name);
    if (pattern) {
      log->set_pattern(*pattern);
    }
    if (level) {
      log->set_level(*level);
    }
  }
}

// --------------------------------------------------------------------------------------------- //

std::string PluginConf::canonical() const {
  // Handle builtins specially, these are in a URI form.
  if (boost::starts_with(plugin_path.string(), "builtin://")) {
    return plugin_path.string();
  }

  try {
    return fs::canonical(plugin_path).native();
  } catch (fs::filesystem_error&) {
    // The plugin_path does not exist in the filesystem or cannot be accessed,
    // in that case we fall back to returning the native representation.
    return plugin_path.native();
  }
}

// --------------------------------------------------------------------------------------------- //

inline auto include_prototype() { return IncludeSchema(nullptr, "").file_exists(); }

Conf default_conf_reader(const std::string& filepath) { return Conf{filepath}; }

Stack::Stack()
    : Confable()
    , reserved_ids_({"_", "cloe", "sim", "simulation"})
    , engine_schema(&engine, "engine configuration")
    , include_schema(&include, include_prototype(), "include configurations")
    , plugins_schema(&plugins, "plugin configuration")
    , vehicle_prototype(nullptr, "")
    , conf_reader_func_(default_conf_reader) {}

Stack::Stack(const Stack& other)
    : Confable(other)
    // Constants (1)
    , reserved_ids_(other.reserved_ids_)
    // Configuration (13)
    , engine(other.engine)
    , server(other.server)
    , include(other.include)
    , logging(other.logging)
    , plugins(other.plugins)
    , simulator_defaults(other.simulator_defaults)
    , simulators(other.simulators)
    , controller_defaults(other.controller_defaults)
    , controllers(other.controllers)
    , component_defaults(other.component_defaults)
    , vehicles(other.vehicles)
    , triggers(other.triggers)
    , simulation(other.simulation)
    // Schemas (3) & Prototypes (3)
    , engine_schema(&engine, "engine configuration")
    , include_schema(&include, include_prototype(), "include configurations")
    , plugins_schema(&plugins, "plugin configuration")
    , simulator_prototype(other.simulator_prototype)
    , controller_prototype(other.controller_prototype)
    , vehicle_prototype(other.vehicle_prototype)
    // State (3)
    , scanned_plugin_paths_(other.scanned_plugin_paths_)
    , all_plugins_(other.all_plugins_)
    , applied_confs_(other.applied_confs_)
    , conf_reader_func_(other.conf_reader_func_) {
  // Reset invalidated schema caches.
  Stack::reset_schema();
}

Stack::Stack(Stack&& other) : Stack() { swap(*this, other); }

Stack& Stack::operator=(Stack other) {
  // Make use of the copy constructor and then swap.
  swap(*this, other);
  return *this;
}

void swap(Stack& left, Stack& right) {
  using std::swap;

  // Constants (1)
  swap(left.reserved_ids_, right.reserved_ids_);

  // Configuration (13)
  swap(left.engine, right.engine);
  swap(left.server, right.server);
  swap(left.include, right.include);
  swap(left.logging, right.logging);
  swap(left.plugins, right.plugins);
  swap(left.simulator_defaults, right.simulator_defaults);
  swap(left.simulators, right.simulators);
  swap(left.controller_defaults, right.controller_defaults);
  swap(left.controllers, right.controllers);
  swap(left.component_defaults, right.component_defaults);
  swap(left.vehicles, right.vehicles);
  swap(left.triggers, right.triggers);
  swap(left.simulation, right.simulation);

  // Prototypes (3)
  swap(left.simulator_prototype, right.simulator_prototype);
  swap(left.controller_prototype, right.controller_prototype);
  swap(left.vehicle_prototype, right.vehicle_prototype);

  // State (3)
  swap(left.scanned_plugin_paths_, right.scanned_plugin_paths_);
  swap(left.all_plugins_, right.all_plugins_);
  swap(left.applied_confs_, right.applied_confs_);
  swap(left.conf_reader_func_, right.conf_reader_func_);

  // Schemas (3)
  // Reset invalidated schema cache.
  left.reset_schema();
  right.reset_schema();
}

void Stack::reset_schema() {
  // clang-format off
  engine_schema = EngineSchema(&engine, "engine configuration");
  include_schema = IncludesSchema(&include, include_prototype(), "include configurations").extend(true);
  plugins_schema = PluginsSchema(&plugins, "plugin configuration").extend(true);
  Confable::reset_schema();
  // clang-format on
}

void Stack::apply_plugin_conf(const PluginConf& c) {
  // 1. Check existence
  if (!fs::exists(c.plugin_path)) {
    if (c.ignore_missing.value_or(engine.plugins_ignore_missing)) {
      logger()->debug("Skip {}", c.plugin_path.native());
      return;
    } else {
      throw Error("plugin path does not exist");
    }
  }

  // 2. Load plugins
  if (fs::is_directory(c.plugin_path)) {
    if (c.plugin_name) {
      throw Error("name can only be specified when path is a file");
    }

    fs::directory_iterator x_end;
    for (fs::directory_iterator x(c.plugin_path); x != x_end; ++x) {
      if (x->path().extension() == ".so") {
        PluginConf xc{c};
        xc.plugin_path = x->path();
        insert_plugin(xc);
      }
    }
  } else {
    insert_plugin(c);
  }
}

void Stack::insert_plugin(const PluginConf& c) {
  const auto canon = c.canonical();
  logger()->debug("Load plugin {}", canon);
  std::shared_ptr<Plugin> plugin;
  try {
    plugin = std::make_shared<Plugin>(canon, c.plugin_name.value_or(""));

    if (!plugin->is_compatible()) {
      if (plugin->is_type_known()) {
        throw PluginError(plugin->path(), "plugin has incompatible version, {} != {}",
                          plugin->type_version(), plugin->required_type_version());
      } else {
        throw PluginError(plugin->path(), "plugin has unknown type, {}", plugin->type());
      }
    }

    insert_plugin(plugin, c);
  } catch (PluginError& e) {
    logger()->error("Error loading plugin {}: {}", canon, e.what());
    if (!c.ignore_failure.value_or(engine.plugins_ignore_failure)) {
      throw;
    }
  }
}

void Stack::insert_plugin(std::shared_ptr<Plugin> p, const PluginConf& c) {
  // Determine short name
  const std::string name = c.plugin_prefix.value_or("") + c.plugin_name.value_or(p->name());

  // Determine canonical name
  std::string canon;
  if (c.plugin_path.empty()) {
    canon = fmt::format("builtin://{}/{}", p->type(), name);
  } else {
    canon = c.canonical();
  }

  // Skip loading if already loaded
  if (all_plugins_.count(canon)) {
    logger()->debug("Skip {}", canon);
    return;
  }
  all_plugins_[canon] = p;

  // Insert into schema
  auto check_insert = [&](auto& data, auto&& f) {
    if (data.has_factory(name)) {
      if (c.allow_clobber.value_or(engine.plugins_allow_clobber)) {
        // The plugin is still available by the path, even if clobbered.
        logger()->warn("Clobber {} with {}", name, canon);
      } else {
        throw PluginError(p->path(),
                          fmt::format("{} already exists with name {}", p->type(), name));
      }
    }
    data.add_factory(name, std::forward<decltype(f)>(f));
  };

  const auto type = p->type();
  if (type == "simulator") {
    check_insert(simulator_prototype, p->make<SimulatorFactory>());
  } else if (type == "controller") {
    check_insert(controller_prototype, p->make<ControllerFactory>());
  } else if (type == "component") {
    check_insert(vehicle_prototype, p->make<ComponentFactory>());
  } else {
    throw PluginError(p->path(), "incompatible plugin type, {}", type);
  }

  // Reset the schema so the new factory is found
  this->reset_schema();
}

bool Stack::has_plugin_with_name(const std::string& key) const {
  for (auto& p : all_plugins_) {
    if (p.second->name() == key) {
      return true;
    }
  }
  return false;
}

bool Stack::has_plugin_with_path(const std::string& plugin_path) const {
  return all_plugins_.count(plugin_path) || all_plugins_.count(fs::canonical(plugin_path).native());
}

std::shared_ptr<Plugin> Stack::get_plugin_with_name(const std::string& key) const {
  for (auto& p : all_plugins_) {
    if (p.second->name() == key) {
      return p.second;
    }
  }
  throw std::out_of_range("no such plugin");
}

std::shared_ptr<Plugin> Stack::get_plugin_with_path(const std::string& plugin_path) const {
  if (all_plugins_.count(plugin_path)) {
    return all_plugins_.at(plugin_path);
  }
  return all_plugins_.at(fs::canonical(plugin_path).native());
}

std::shared_ptr<Plugin> Stack::get_plugin_or_load(const std::string& key_or_path) const {
  if (has_plugin_with_name(key_or_path)) {
    return get_plugin_with_name(key_or_path);
  } else if (has_plugin_with_path(key_or_path)) {
    return get_plugin_with_path(key_or_path);
  } else {
    // Try to load it myself, temporarily.
    return std::make_shared<Plugin>(fs::canonical(key_or_path).native());
  }
}

namespace {

std::vector<DefaultConf> get_defaults(const std::vector<DefaultConf>& defaults, std::string binding,
                                      std::string name) {
  std::vector<DefaultConf> output;
  for (const auto& c : defaults) {
    if (c.name.value_or(name) == name && c.binding.value_or(binding) == binding) {
      output.push_back(c);
    }
  }
  return output;
}

}  // anonymous namespace

std::vector<DefaultConf> Stack::get_simulator_defaults(std::string b, std::string n) const {
  return get_defaults(simulator_defaults, b, n);
}

std::vector<DefaultConf> Stack::get_controller_defaults(std::string b, std::string n) const {
  return get_defaults(controller_defaults, b, n);
}

std::vector<DefaultConf> Stack::get_vehicle_defaults(std::string) const {
  return std::vector<DefaultConf>{};
}

std::vector<DefaultConf> Stack::get_component_defaults(std::string b, std::string n) const {
  return get_defaults(component_defaults, b, n);
}

Json Stack::active_config() const {
  Json j;
  to_json(j);
  return j;
}

Json Stack::input_config() const {
  Json j = Json::array();
  for (const auto& c : applied_confs_) {
    j.emplace_back(Json{
        {"file", c.is_from_file() ? c.file() : "-"},
        {"data", *c},
    });
  }
  return j;
}

void Stack::to_json(Json& j) const {
  Confable::to_json(j);

  // Because the merged stack already has the includes, we don't print them
  // in the default representation.
  if (j.count("include")) {
    j.erase("include");
  }
}

void Stack::from_conf(const Conf& _conf, size_t depth) {
  applied_confs_.emplace_back(_conf);
  Conf c = _conf;

  // First check the version so the user gets higher-level errors first.
  if (c.has("version")) {
    auto ver = c.get<std::string>("version");
    if (ver != CLOE_STACK_VERSION) {
      // If we threw a SchemaError, it would look like this:
      //
      //     throw SchemaError{
      //       c,
      //       this->schema().json_schema(),
      //       Json{},
      //       "require version {}, got {}",
      //       CLOE_STACK_VERSION,
      //       ver,
      //     };
      //
      // But that would result in a very verbose error message, so we shall
      // throw a more user-friendly error message instead.
      throw Error{"require version {}, got {}", CLOE_STACK_VERSION, ver}.explanation(R"(
            It looks like you are attempting to load a stack file with an
            incompatible version.

            You have two choices for proceeding:

              a) Migrate the stack file to the current version.
              b) Use another version of the Cloe runtime.

            Migrating from an earlier to a later version can sometimes be
            automated, please see the Cloe CLI for more details.
            )");
    }
  } else {
    // clang-format off
    throw Error{"require version property"}.explanation(fmt::format(R"(
          It looks like you are attempting to load a stack file that does not
          have a version specified.

          This is required, so that the Cloe runtime knows which schema to use
          for data deserialization. It is good practice to place the version
          field at the top of the JSON stack file:

            {{
              "version": "{}",
              ...
            }}
          )", CLOE_STACK_VERSION));
    // clang-format on
  }

  if (c.has_pointer("/engine/ignore")) {
    auto v = c.get_pointer<std::vector<std::string>>("/engine/ignore");
    for (const auto& s : v) {
      engine.ignore_sections.emplace_back(s);
    }
    c.erase_pointer("/engine/ignore");
  }

  for (const auto& s : engine.ignore_sections) {
    if (c.has_pointer(s)) {
      logger()->info("Ignoring {}:{}", c.file(), s);
      c.erase_pointer(s);
    }
  }

  // Apply engine configuration first, since they affect include depth and plugin
  // loading.
  if (c.has("engine")) {
    engine_schema.from_conf(c.at("engine"));
    c.erase("engine");
  }

  // Apply include configurations, in case some define plugins we need.
  if (c.has("include")) {
    size_t k = include.size();
    include_schema.from_conf(c.at("include"));
    size_t n = include.size();
    for (size_t i = k; i < n; i++) {
      auto filepath = include[i].native();
      logger()->info("Include conf: {}", filepath);
      if (depth > engine.security_max_include_depth) {
        throw Error{"maximum include recursion depth reached: {}", depth}.explanation(R"(
              Cloe limits the amount of times you can include other stack files from
              within stackfiles. This is to protect you from cyclic dependencies
              causing an infinite loop, which could otherwise crash Cloe or make the
              system unstable.

              The most likely cause is that you have a cyclic dependency in your
              includes. Try running Cloe with increased logging verbosity to see
              which files are being included.

              If you feel that you need more than the default allowed recursion
              depth (64), you are free to increase the limit within the stack file:

                {{
                  "engine": {{
                    "security": {{
                      "max_include_depth": 1024
                    }}
                  }},
                  ...
                }}

              This should be done sparingly. If you have such an inclusion depth,
              chances are the structure of the stack files is sub-optimal.
              )");
      }
      Conf config;
      try {
        config = conf_reader_func_(filepath);
      } catch (std::exception& e) {
        auto jp = fmt::format("/include/{}", i);
        c.at_pointer(jp).throw_error(e.what());
      }
      from_conf(config, depth + 1);
    }
    c.erase("include");
  }

  // Re-apply the engine configuration at this point, so that a higher-level
  // engine configuration overrides any settings an included configuration
  // might have made.
  if (c.has("engine")) {
    engine_schema.from_conf(_conf.at("engine"));
  }

  // Load default plugins before explicitly specified plugins.
  for (auto p : engine.plugin_path) {
    // This section is run for each stack file we include, and so the plugin
    // path might contain duplicate entries in the end, since it is filled by
    // default. We want to avoid that, so we keep track of which plugin paths
    // we have already scanned for plugins.
    if (scanned_plugin_paths_.count(p)) {
      continue;
    }
    scanned_plugin_paths_.insert(p);

    // Scan plugin path for plugins.
    PluginConf x;
    x.plugin_path = p;
    x.ignore_missing = true;
    x.allow_clobber = false;
    apply_plugin_conf(x);
  }

  // Apply plugin configuration, since this will load controller, simulator,
  // and component schemas.
  if (c.has("plugins")) {
    size_t n = plugins.size();
    plugins_schema.from_conf(c.at("plugins"));
    for (size_t i = n; i < plugins.size(); i++) {
      logger()->debug("Insert plugin {}", plugins[i].plugin_path.native());
      try {
        apply_plugin_conf(plugins[i]);
      } catch (Error& e) {
        auto ec = c.at("plugins").to_array()[i];
        throw SchemaError{ec, plugins_schema.json_schema(), Json{}, e.what()};
      }
    }
    c.erase("plugins");
  }

  // Apply everything else.
  this->schema().validate(c);
  this->schema().from_conf(c);
  this->reset_schema();
}

void Stack::validate(const Conf& c) const {
  Stack copy(*this);
  copy.from_conf(c);
  copy.validate();
}

void Stack::validate() const {
  check_consistency();
  check_defaults();
}

bool Stack::is_valid() const {
  try {
    validate();
    return true;
  } catch (...) {
    // TODO(ben): Replace ... with specific error classes
    return false;
  }
}

void Stack::check_consistency() const {
  std::map<std::string, std::string> ns;
  for (auto x : reserved_ids_) {
    ns[x] = "reserved keyword";
  }

  /**
   * Check that the given name does not exist yet.
   */
  auto check = [&](const char* type, const std::string& key) {
    if (ns.count(key)) {
      throw Error("cannot define a new {} with the name '{}': a {} with that name already exists",
                  type, key, ns[key]);
    }
    ns[key] = type;
  };

  auto check_has = [&](const char* type, const std::string& key) {
    if (!ns.count(key)) {
      throw Error("cannot find a {} with the name '{}': no entity with that name has been defined",
                  type, key);
    } else if (ns[key] != type) {
      throw Error("cannot find a {} with the name '{}': a {} with that name already exists", type,
                  key, ns[key]);
    }
  };

  for (const auto& x : simulators) {
    check("simulator", x.name.value_or(x.binding));
  }
  for (const auto& x : vehicles) {
    check("vehicle", x.name);
    if (x.is_from_vehicle()) {
      // If this vehicle depends on another vehicle, that vehicle *must* have
      // been defined already in the list. We don't do any dependency
      // resolution yet.
      check_has("vehicle", x.from_veh);
    } else {
      // We can check whether the simulator exists, but it's not possible
      // during configuration to actually know whether the simulator in
      // question will cough up the vehicle we want.
      check_has("simulator", x.from_sim.simulator);
    }
    for (const auto& kv : x.components) {
      check("component", kv.second.name.value_or(kv.second.binding));
    }
  }
  for (const auto& x : controllers) {
    check("controller", x.name.value_or(x.binding));
    check_has("vehicle", x.vehicle);
  }
}

void Stack::check_defaults() const {
  auto check = [&](auto f, const std::string& name, const std::vector<DefaultConf> defaults) {
    auto y = f->clone();
    for (const auto& c : defaults) {
      if (c.name.value_or(name) == name && c.binding.value_or(f->name()) == f->name()) {
        y->from_conf(c.args);
      }
    }
  };

  for (const auto& x : simulators) {
    check(x.factory, x.name.value_or(x.binding), simulator_defaults);
  }
  for (const auto& x : controllers) {
    check(x.factory, x.name.value_or(x.binding), controller_defaults);
  }
  for (const auto& x : vehicles) {
    for (const auto& kv : x.components) {
      check(kv.second.factory, kv.second.name.value_or(kv.second.binding), component_defaults);
    }
  }
}

bool Stack::is_complete() const {
  return !simulators.empty() && !vehicles.empty() && !controllers.empty();
}

void Stack::check_completeness() const {
  std::vector<std::string> missing;
  if (simulators.empty()) {
    missing.emplace_back("simulators");
  }
  if (vehicles.empty()) {
    missing.emplace_back("vehicles");
  }
  if (controllers.empty()) {
    missing.emplace_back("controllers");
  }
  if (!missing.empty()) {
    throw StackIncompleteError(std::move(missing));
  }
}

}  // namespace cloe
