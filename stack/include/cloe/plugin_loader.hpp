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
 * \file cloe/plugin_laoder.hpp
 * \see  cloe/plugin_loader.cpp
 */

#pragma once

#include <functional>  // for function<>
#include <memory>      // for unique_ptr<>
#include <string>      // for string

#include <cloe/core.hpp>    // for Json, Error
#include <cloe/plugin.hpp>  // for PluginManifest

namespace cloe {

/**
 * PluginError is the superclass of errors that can occur during plugin loading
 * and handling.
 */
class PluginError : public Error {
 public:
  PluginError(const std::string& path, const std::string& what) : Error(what), plugin_path_(path) {}
  PluginError(const std::string& path, const char* what) : Error(what), plugin_path_(path) {}

  template <typename... Args>
  PluginError(const std::string& path, const char* format, const Args&... args)
      : Error(format, args...), plugin_path_(path) {}

  virtual ~PluginError() noexcept = default;

  /**
   * Return the plugin path.
   *
   * - If plugin is builtin, returned string is empty.
   */
  const std::string& plugin_path() const { return plugin_path_; }

 private:
  std::string plugin_path_;
};

class Plugin {
 public:
  /**
   * Construct a Plugin by loading a dynamic library from disk.
   *
   * The following call is used to load the plugin:
   *
   *    dlopen(plugin_path, RTLD_GLOBAL | RTLD_DEEPBIND | RTLD_NOW)
   *
   * **Discussion**
   *
   * Currently, this opens a plugin with the dlopen() call (provided by glibc).
   * This constrains official support to Linux.
   *
   * There are several ways we can open a plugin. Ideally, we would use:
   *
   *    dlmopen(LM_ID_NEWLM, plugin_path, RTLD_GLOBAL | RTLD_NOW)
   *
   * This would open each plugin within its own namespace, which prevents one
   * plugin from affecting another plugin. However, glibc does not support the
   * use of the RTLD_GLOBAL mode in dlmopen(), and there is currently only
   * support for up to 16 namespaces. See the following RFC:
   *
   *    RFC: Treat RTLD_GLOBAL as unique to namespace when used with dlmopen
   *    https://patchwork.ozlabs.org/project/glibc/patch/55A73673.3060104@redhat.com/
   *
   * Unfortunately, this RFC hasn't seen much activity since 2015, so it is
   * unlikely to merged soon, and even if it was, we wouldn't be able to use
   * it until all platforms and distributions we support have that version.
   *
   * The alternative to using dlmopen() is to use the non-namespaced dlopen():
   *
   *    dlopen(plugin_path, RTLD_LOCAL | RTLD_NOW)
   *
   * With the local scope as achieved with RTLD_LOCAL, we prevent plugins from
   * interfering with each other, but there is still a major problem with this
   * approach: dependencies of a plugin are prevented from sharing symbols,
   * which leads to runtime errors. Fixing this requires RTLD_GLOBAL:
   *
   *    dlopen(plugin_path, RTLD_GLOBAL | RTLD_NOW)
   *
   * This causes problems combining plugins that include conflicting versions
   * of libraries though, so we need to add the RTLD_DEEPLINK flag to cause
   * libraries to prefer local symbols during resolution.
   *
   * Note: The use of RTLD_NOW as opposed to RTLD_LAZY makes loader errors
   * occur when we load the plugin as opposed to when the plugin is invoked.
   * This comes at a performance penalty as all plugins will be loaded
   * regardless of whether they are used. However, this moves errors to a point
   * before a simulation occurs, which is preferable. In the future, we may
   * lazily load plugins and then load them again once it is clear they will
   * partake in the simulation.
   */
  explicit Plugin(const std::string& plugin_path, const std::string& name = "");

  /**
   * Construct a Plugin from a Plugin-compatible type itself.
   *
   * This does not involve dlopen().
   */
  Plugin(const PluginManifest& m, std::function<ModelFactory*()> fn, const std::string& name = "");

  /**
   * Return the path to the loaded dynamic library.
   *
   * - Response may be empty if plugin is built-in.
   */
  std::string path() const { return path_; }

  /**
   * Return the given or intrinsic name of the plugin.
   */
  std::string name() const { return name_; }

  /**
   * Return the plugin type.
   */
  std::string type() const { return manifest_.plugin_type; }

  /**
   * Return the API version of the plugin.
   */
  std::string type_version() const { return manifest_.plugin_type_version; }

  /**
   * Return the version that the Cloe library expects the plugin to have.
   */
  std::string required_type_version() const;

  /**
   * Return the schema of this plugin.
   */
  Schema schema() const;

  /**
   * Return whether this plugin is builtin (as opposed to loaded from disk).
   */
  bool is_builtin() const;

  /**
   * Return whether this plugin type is known to Cloe.
   */
  bool is_type_known() const;

  /**
   * Return whether this plugin is compatible with Cloe.
   *
   * This should be checked before creating any objects with the factory
   * function from the plugin. Deviation results in undefined behavior.
   */
  bool is_compatible() const;

  /**
   * Attempt to cast this component to a sub-type.
   *
   * - Throws an exception if the component cannot be cast.
   */
  template <typename F>
  std::unique_ptr<F> make() const {
    if (!is_compatible()) {
      throw PluginError(path(), "cannot make factory from incompatible plugin");
    }

    auto f = std::unique_ptr<F>(dynamic_cast<F*>(createf_()));
    f->set_name(name());
    return f;
  }

  friend void to_json(Json& j, const Plugin& p) {
    j = Json{
        {"path", p.path()},
        {"name", p.name()},
        {"type", p.type()},
        {"type_version", p.type_version()},
        {"is_known_type", p.is_type_known()},
        {"is_compatible", p.is_compatible()},
    };
  }

 private:
  std::string path_;
  std::string name_;
  PluginManifest manifest_;
  void* handle_;
  std::function<ModelFactory*()> createf_;
};

/**
 * Create a virtual plugin around built-in types.
 */
template <typename F>
std::shared_ptr<Plugin> make_plugin() {
  PluginManifest manifest{
      BASE_CLOE_FACTORY(F)::PLUGIN_TYPE,
      BASE_CLOE_FACTORY(F)::PLUGIN_API_VERSION,
      nullptr,
      0,
  };
  auto fn = []() { return new F(); };
  return std::make_shared<Plugin>(manifest, fn);
}

}  // namespace cloe
