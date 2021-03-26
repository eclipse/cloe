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
 * \file cloe/plugin.hpp
 * \see  cloe/version.hpp
 *
 * This file enables controllers, simulators, and components to be easily used
 * as plugins.
 *
 * The library is never unloaded, so we can forget about the handle.  It does
 * mean that we leak it in the end though, so that might show up in valgrind or
 * similar tools.
 *
 * All controller plugins that we load should be compiled to the correct
 * version. This is a precaution that will probably save us a lot of grief in
 * the long run IF we keep bumping the versions found in <cloe/version.hpp>.
 *
 * # Debugging Plugins
 *
 * If you run into any troubles when loading a plugin, make sure that the
 * appropriate symbols are exported:
 *
 *   $ nm your_plugin.so | grep -e 'plugin_manifest\|create_factory'
 *   0000000000027949 T create_factory
 *   000000000033ba30 D plugin_manifest
 *
 * If that looks good, then possibly you need to recompile your plugin
 * with the same version of Cloe that you are trying to load it with.
 */

#pragma once
#ifndef CLOE_PLUGIN_HPP_
#define CLOE_PLUGIN_HPP_

#include <type_traits>  // for is_base_of, conditional<>, false_type

/*
 * When creating a plugin, only the symbols that are loaded should be made
 * visible. This can be achieved by setting compile options:
 *
 *     -fvisibility=hidden -fvisibility-inlines-hidden
 *
 * This macro then makes select symbols that we want to export visible again.
 */
#define DLL_PUBLIC __attribute__((visibility("default")))

/*
 * This definition is used for tracking changes to the plugin manifest.
 */
#define CLOE_PLUGIN_MANIFEST_VERSION 1

/*
 * This can be set to a compatible setting as defined in the system header
 * dlfcn.h. The default is equivalent to RTLD_LOCAL. Prefer defining this
 * with the defines from the system header rather than on the command line.
 *
 * Note that if set to anything other than 0, one of RTLD_NOW and RTLD_LAZY
 * must be specified in addition, as per the manual of dlopen().
 *
 * Example: RTLD_NOLOAD | RTLD_GLOBAL | RTLD_LAZY
 */
#ifndef CLOE_PLUGIN_GLIBC_DLOPEN_MODE
#define CLOE_PLUGIN_GLIBC_DLOPEN_MODE 0
#endif

/*
 * This complex looking beast of code basically contains two switches on the
 * base type of the factory xFactoryType. Ideally, we'd like to be able to
 * just be able to do this:
 *
 *     xFactoryType::PLUGIN_TYPE,
 *
 * But PLUGIN_TYPE and PLUGIN_API_VERSION are static members of the base
 * factories, and are thus not available on the xFactoryType. So first we
 * find out through is_base_of and conditional.
 */
#define BASE_CLOE_FACTORY(xFactoryType)                                                    \
  std::conditional<std::is_base_of<::cloe::ControllerFactory, xFactoryType>::value,        \
                   ::cloe::ControllerFactory,                                              \
                   typename std::conditional<                                              \
                       std::is_base_of<::cloe::SimulatorFactory, xFactoryType>::value,     \
                       ::cloe::SimulatorFactory,                                           \
                       typename std::conditional<                                          \
                           std::is_base_of<::cloe::ComponentFactory, xFactoryType>::value, \
                           ::cloe::ComponentFactory, std::false_type>::type>::type>::type

/**
 * This macro makes the given factory available to Cloe as a plugin.
 *
 * # Example
 *
 * Given your component binding FooBar with its factory FooBarFactory, you can,
 * in your main source file, export it as a plugin for Cloe:
 *
 *     namespace foobar {
 *
 *     class FooBarFactory : public ControllerFactory {
 *       // ... implementation ...
 *     };
 *
 *     } // namespace foobar
 *
 *     EXPORT_CLOE_PLUGIN(foobar::FooBarFactory)
 *
 * The macro call should not be followed by a semicolon, it should appear in
 * the global namespace, and it should only be called on classes that inherit
 * from one of the supported plugin types: ControllerFactory, SimulatorFactory,
 * and ComponentFactory. The macro argument should be a fully-qualified name
 * of the factory.
 */
#define EXPORT_CLOE_PLUGIN(xFactoryType)                                                           \
  extern "C" DLL_PUBLIC const uint8_t cloe_plugin_manifest_version = CLOE_PLUGIN_MANIFEST_VERSION; \
  extern "C" DLL_PUBLIC const ::cloe::PluginManifest cloe_plugin_manifest{                         \
      BASE_CLOE_FACTORY(xFactoryType)::PLUGIN_TYPE,                                                \
      BASE_CLOE_FACTORY(xFactoryType)::PLUGIN_API_VERSION,                                         \
      "cloe_plugin_create",                                                                        \
      CLOE_PLUGIN_GLIBC_DLOPEN_MODE,                                                               \
  };                                                                                               \
  extern "C" DLL_PUBLIC ::cloe::ModelFactory* cloe_plugin_create() { return new xFactoryType{}; }

namespace cloe {

// Forward declarations:
class ControllerFactory;
class SimulatorFactory;
class ComponentFactory;
class ModelFactory;

/**
 * PluginManifest is the C struct that can be dynamically loaded and which
 * defines the interface between the Cloe engine and a Cloe plugin.
 *
 * See the EXPORT_CLOE_PLUGIN macro for more information on how to define
 * this struct.
 */
struct PluginManifest {
  /**
   * The plugin type is one of: component, controller, simulator.
   */
  const char* plugin_type;

  /**
   * The plugin API version is defined by the plugin type above.
   */
  const char* plugin_type_version;

  /**
   * The factory symbol defines which function should be used to create the
   * model factory.
   */
  const char* factory_symbol;

  /**
   * The dlopen is RTLD_LOCAL | RTLD_NOW by default but can be overridden here.
   *
   * See the CLOE_PLUGIN_GLIBC_DLOPEN_MODE define for information on how to
   * configure this.
   *
   * \since CLOE_PLUGIN_VERSION == 1
   * \since cloe-runtime == 0.18.0
   */
  int glibc_dlopen_mode;
};

}  // namespace cloe

#endif  // CLOE_PLUGIN_HPP_
