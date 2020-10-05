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
 * \file plugin.cpp
 * \see  plugin.hpp
 */

#include "plugin.hpp"

#include <dlfcn.h>  // for dlopen, dlsym, RTLD_NOW, ...
#include <memory>   // for unique_ptr<>
#include <set>      // for set<>
#include <string>   // for string

#include <cloe/component.hpp>   // for ComponentFactory
#include <cloe/controller.hpp>  // for ControllerFactory
#include <cloe/model.hpp>       // for ModelFactory
#include <cloe/plugin.hpp>      // for PluginManifest
#include <cloe/simulator.hpp>   // for SimulatorFactory

namespace cloe {

namespace {

// It's easier to support old versions of the plugin manifest than it is to
// deal with support tickets of segfaults that happen during loading.

uint8_t read_plugin_manifest_version(void* handle) {
  uint8_t* sym = reinterpret_cast<uint8_t*>(dlsym(handle, "cloe_plugin_manifest_version"));
  if (sym == nullptr) {
    // There is no symbol. This is to be expected for cloe_plugin_manifest_version == 0.
    return 0;
  }
  return *sym;
}

struct PluginManifestV0 {
  const char* plugin_type;
  const char* plugin_version;
  const char* factory_symbol;
};

PluginManifest read_plugin_manifest_v0(void* handle, const std::string& plugin_path) {
  auto manifest = reinterpret_cast<const PluginManifestV0*>(dlsym(handle, "plugin_manifest"));
  if (manifest == nullptr) {
    throw PluginError(plugin_path, "expected symbol 'plugin_manifest' not found");
  }

  // Before we do anything further with the plugin, make sure it is not one of
  // the versions we shouldn't even load. If it's not compatible, we shouldn't
  // load it.
  //
  // Note that this is not the same as the plugin version, which only describes
  // the manifest.
  if (manifest->plugin_version[0] == '0') {
    throw PluginError(plugin_path, "incompatible plugin API version: {}", manifest->plugin_version);
  }

  return PluginManifest{
      manifest->plugin_type,
      manifest->plugin_version,
      manifest->factory_symbol,
      RTLD_LOCAL,
  };
}

struct PluginManifestV1 {
  const char* plugin_type;
  const char* plugin_type_version;
  const char* factory_symbol;
  int glibc_dlopen_mode;
};

PluginManifest read_plugin_manifest_v1(void* handle, const std::string& plugin_path) {
  auto manifest = reinterpret_cast<const PluginManifestV1*>(dlsym(handle, "cloe_plugin_manifest"));
  if (manifest == nullptr) {
    throw PluginError(plugin_path, "expected symbol 'cloe_plugin_manifest' not found");
  }
  return PluginManifest{
      manifest->plugin_type,
      manifest->plugin_type_version,
      manifest->factory_symbol,
      manifest->glibc_dlopen_mode,
  };
}

PluginManifest read_plugin_manifest(void* handle, const std::string& plugin_path) {
  auto version = read_plugin_manifest_version(handle);
  switch (version) {
    case 0:
      return read_plugin_manifest_v0(handle, plugin_path.c_str());
    case 1:
      return read_plugin_manifest_v1(handle, plugin_path.c_str());
    default:
      throw PluginError(plugin_path, "incompatible plugin version: {}", version);
  }
}

const std::set<std::string> PLUGIN_TYPES_KNOWN({
    "component",
    "controller",
    "simulator",
});

}  // anonymous namespace

Plugin::Plugin(const std::string& plugin_path, const std::string& name)
    : path_(plugin_path), name_(name) {
  // Load the plugin with a very conservative mode.
  // This will called again after we have the plugin manifest.
  handle_ = dlopen(plugin_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
  if (handle_ == nullptr) {
    throw PluginError(plugin_path, dlerror());
  }

  // Get the manifest.
  manifest_ = read_plugin_manifest(handle_, plugin_path);

  // If the plugin manifest defines different loader settings, apply those now.
  if (manifest_.glibc_dlopen_mode != 0) {
    int mode = manifest_.glibc_dlopen_mode;
    auto log = logger::get("cloe");

    log->debug("{}: Overriding GLIBC dlopen() mode: {}", plugin_path, mode);
    dlclose(handle_);
    handle_ = dlopen(plugin_path.c_str(), mode);
    if (handle_ == nullptr) {
      throw PluginError(plugin_path, dlerror());
    }
  }

  // Get the factory creator handle.
  auto factory_fn = dlsym(handle_, manifest_.factory_symbol);
  assert(factory_fn != nullptr);
  createf_ = reinterpret_cast<ModelFactory* (*)()>(factory_fn);

  // Get the factory name if none is specified.
  if (name_.empty()) {
    std::unique_ptr<ModelFactory> factory{createf_()};
    name_ = factory->name();
  }
}

Plugin::Plugin(const PluginManifest& m, std::function<ModelFactory*()> fn, const std::string& name)
    : path_(), name_(name), manifest_(m), handle_(nullptr), createf_(fn) {
  if (name_.empty()) {
    auto tmpf = std::unique_ptr<ModelFactory>(createf_());
    name_ = tmpf->name();
  }
}

Schema Plugin::schema() const {
  auto tmpf = std::unique_ptr<ModelFactory>(createf_());
  return Schema{tmpf->schema()}.reset_pointer();
}

bool Plugin::is_type_known() const { return PLUGIN_TYPES_KNOWN.count(type()); }

bool Plugin::is_compatible() const {
  return type_version() == required_type_version() && is_type_known();
}

std::string Plugin::required_type_version() const {
  if (type() == "controller") {
    return ControllerFactory::PLUGIN_API_VERSION;
  } else if (type() == "simulator") {
    return SimulatorFactory::PLUGIN_API_VERSION;
  } else if (type() == "component") {
    return ComponentFactory::PLUGIN_API_VERSION;
  } else {
    return "invalid";
  }
}

}  // namespace cloe
