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
 * \file config.hpp
 */

#pragma once

#ifndef CLOE_CONTACT_EMAIL
#define CLOE_CONTACT_EMAIL "cloe-dev@eclipse.org"
#endif

#ifndef CLOE_STACK_VERSION
#define CLOE_STACK_VERSION "4.1"
#endif

#ifndef CLOE_STACK_SUPPORTED_VERSIONS
#define CLOE_STACK_SUPPORTED_VERSIONS {"4", "4.0", "4.1"}
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

#ifndef CLOE_SIMULATION_UUID_VAR
#define CLOE_SIMULATION_UUID_VAR "CLOE_SIMULATION_UUID"
#endif

// The environment variable from which additional plugins should
// be loaded. Takes the same format as PATH.
#ifndef CLOE_PLUGIN_PATH
#define CLOE_PLUGIN_PATH "CLOE_PLUGIN_PATH"
#endif

