--
-- Copyright 2023 Robert Bosch GmbH
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--
-- SPDX-License-Identifier: Apache-2.0
--

---
--- @meta cloe-engine.types
---
--- This file contains the type annotations of the `cloe-engine.types` module,
--- which are exported by the cloe-engine executable.
---
--- These methods should only be used by the cloe library.
---

--- @class Stack
local Stack = {}

--- @class InputConf applied input stack configuration
--- @field file string source of stack, can be "" if unknown or "-" if stdin
--- @field data StackConf the contents of the input stack configuration

--- @class StackConf stack configuration
--- @field version string version of stack (should be "4")
--- @field include? string[] list of files to include
--- @field engine? EngineConf engine configuration
--- @field simulation? SimulationConf simulation configuration
--- @field server? ServerConf server configuration
--- @field plugins? PluginConf[] list of plugin configurations
--- @field defaults? DefaultsConf default arguments to apply to plugins
--- @field simulators? SimulatorConf[] simulator configuration
--- @field controllers? ControllerConf[] controller configuration
--- @field vehicles? VehicleConf[] vehicle configuration
--- @field triggers? TriggerConf[] triggers to schedule

--- @class EngineConf
--- @field hooks? { pre_connect?: CommandSpec[], post_disconnect?: CommandSpec[] }
--- @field ignore? string[] fields to ignore in input
--- @field keep_alive? boolean whether to keep cloe-engine alive after simulation end
--- @field output? EngineOutputConf output configuration
--- @field plugin_path? string[] list of plugin files to load
--- @field plugins? { allow_clobber?: boolean, ignore_failure?: boolean, ignore_missing?: boolean }
--- @field polling_interval? number how many milliseconds to wait in pause state (default: 100)
--- @field registry_path? string path to use for registry (where output is also written)
--- @field security? EngineSecurityConf security configuration
--- @field triggers? EngineTriggerConf trigger configuration
--- @field watchdog? EngineWatchdogConf watchdog configuration

--- @class EnginePluginConf
--- @field allow_clobber? boolean whether to allow a plugin to override a previously loaded plugin
--- @field ignore_failure? boolean whether to ignore plugin loading failure (e.g. not a cloe plugin)
--- @field ignore_missing? boolean whether to ignore plugins that are specified but missing

--- @class EngineTriggerConf
--- @field ignore_source? boolean whether to ignore the "source" field into account

--- @class EngineOutputConf
--- @field path? string directory prefix for all output files (relative to registry_path)
--- @field clobber? boolean whether to overwrite pre-existing files (default: true)
--- @field files? EngineOutputFilesConf configuration for each output file

--- @class EngineOutputFilesConf
--- @field config? string simulation configuration (result of defaults and loaded configuration)
--- @field result? string simulation result and report
--- @field triggers? string list of applied triggers
--- @field signals? string list of signals
--- @field signals_autocompletion? string signal autocompletion file for Lua
--- @field api_recording? string data stream recording file

--- @class EngineSecurityConf
--- @field enable_command_action? boolean whether to allow commands (default: true)
--- @field enable_hooks_section? boolean whether to allow hooks to run (default: true)
--- @field enable_include_section? boolean whether to allow files to include other files (default: true)
--- @field max_include_depth? number how many includes deep we can do before aborting (default: 64)

--- @class EngineWatchdogConf
--- @field default_timeout? number in [milliseconds]
--- @field mode? string one of "off", "log", "abort", "kill" (default: "off")
--- @field state_timeouts? table<string, number> timeout values for specific engine states
---
--- @class LoggingConf
--- @field name string name of logger, e.g. "cloe"
--- @field pattern? string pattern to use for logging output
--- @field level? string one of "debug", "trace", "info", "warn", "error", "critical"

--- @class ServerConf
--- @field listen? boolean whether to enable the server (default: true)
--- @field listen_address? string address to listen on (default: "127.0.0.1")
--- @field listen_port? number port to listen on (default: 8080)
--- @field listen_threads? number threads to use (deprecated)
--- @field api_prefix? string endpoint prefix for API endpoints (default: "/api")
--- @field static_prefix? string endpoint prefix for static assets (default: "")

--- @class PluginConf
--- @field path string path to plugin or directory to load
--- @field name? string name to load plugin as (used with binding field later)
--- @field prefix? string apply prefix to plugin name (useful for directories)
--- @field ignore_missing? boolean ignore plugin if missing
--- @field ignore_failure? boolean ignore plugin if cannot load
--- @field allow_clobber? boolean allow plugin to overwrite previously loaded of same name

--- @class SimulatorConf
--- @field binding string plugin name
--- @field name? string simulator name, defaults to plugin name
--- @field args? table simulator configuration (plugin specific)

--- @class TriggerConf
--- @field action string|table|fun(sync: Sync):(boolean?)
--- @field event string|table
--- @field label? string
--- @field source? string
--- @field sticky? boolean
--- @field conceal? boolean
--- @field optional? boolean
--- @field group? string

--- @class VehicleConf
--- @field name string vehicle name, used in controller configuration
--- @field from VehicleFromSimConf|string vehicle data source
--- @field components? table<string, ComponentConf> component configuration

--- @class VehicleFromSimConf
--- @field simulator string simulator name
--- @field index? number vehicle index
--- @field name? string vehicle name

--- @class ControllerConf
--- @field binding string plugin name
--- @field name? string controller name, defaults to plugin name
--- @field vehicle string vehicle to attach to (name in vehicle conf)
--- @field args? table controller configuration (plugin specific)

--- @class SimulationConf
--- @field abort_on_controller_failure? boolean whether to abort when controller fails (default: true)
--- @field controller_retry_limit? number how many times to let controller attempt to make progress (default: 1024)
--- @field controller_retry_sleep? number how long to wait between controller attempts, in [milliseconds]
--- @field model_step_width? number how long a single cycle lasts in the simulation, in [nanoseconds]

--- @class ComponentConf
--- @field binding string plugin name
--- @field from string[]|string source components to use as input
--- @field name? string name to use for component, defaults to plugin name
--- @field args? table component configuration (plugin specific)

--- @class DefaultConf
--- @field name? string name to match
--- @field binding? string binding to match
--- @field args table default arguments to apply (can be overridden)

--- @class DefaultsConf
--- @field components DefaultConf[] defaults for components
--- @field simulators DefaultConf[] defaults for simulators
--- @field controllers DefaultConf[] defaults for controllers

--- Merge JSON stackfile into simulation configuration.
---
--- @param filepath string
--- @return nil
function Stack:merge_stackfile(filepath) end

--- Merge JSON string into simulation configuration.
---
--- @param json string Input JSON (use Lua multiline feature)
--- @param source_filepath string Filepath to use for error messages
--- @return nil
function Stack:merge_stackjson(json, source_filepath) end

--- Merge Lua table into simulation configuration.
---
--- This converts the table to JSON, then loads it.
---
--- @param tbl StackConf Input JSON as Lua table
--- @param source_filepath string Filepath to use for error messages
--- @return nil
function Stack:merge_stacktable(tbl, source_filepath) end

--- Return the current active configuration of the stack file.
---
--- This is not the same thing as the input configuration!
---
--- @return StackConf
function Stack:active_config() end

--- Return an array of input configuration of the stack file.
---
--- This is not the same thing as the active configuration!
---
--- @return InputConf[]
function Stack:input_config() end

--- @class Duration
local Duration = {}

--- Return new Duration instance from duration format.
---
--- @param format string Duration such as "1s" or "1.5 ms"
--- @return Duration
function Duration.new(format) end

--- Return Duration as nanoseconds.
---
--- @return number nanoseconds
function Duration:ns() end

--- Return Duration as microseconds.
---
--- @return number microseconds
function Duration:us() end

--- Return Duration as milliseconds.
---
--- @return number milliseconds
function Duration:ms() end

--- Return Duration as seconds.
---
--- @return number seconds
function Duration:s() end

--- @class Sync
local Sync = {}

--- Return current simulation step.
---
--- @return integer
--- @nodiscard
function Sync:step() end

--- Return simulation step_width.
---
--- @return Duration
--- @nodiscard
function Sync:step_width() end

--- Return current simulation time.
---
--- @return Duration
--- @nodiscard
function Sync:time() end

--- Return estimated simulation end.
---
--- If unknown, then 0 is returned.
---
--- @return Duration
--- @nodiscard
function Sync:eta() end

--- Return current simulation realtime-factor target.
---
--- @return number
--- @nodiscard
function Sync:realtime_factor() end

--- Return whether realtime-factor target is unlimited.
---
--- If true, then the simulation runs as fast as possible and never pads
--- cycles with waiting time.
---
--- @return boolean
--- @nodiscard
function Sync:is_realtime_factor_unlimited() end

--- Return estimated achievable simulation realtime-factor target.
---
--- @return number
--- @nodiscard
function Sync:achievable_realtime_factor() end

--- @class Coordinator
local Coordinator = {}

--- Insert a trigger into the coordinator event queue.
---
--- @param trigger TriggerConf trigger schema to insert
--- @return nil
function Coordinator:insert_trigger(trigger) end

--- Execute an action known to the coordinator immediately.
---
--- @param action string|table action schema to insert
--- @return nil
function Coordinator:execute_action(action) end

--- @enum LogLevel
local LogLevel = {
    TRACE = "trace",
    DEBUG = "debug",
    INFO = "info",
    WARN = "warn",
    ERROR = "error",
    CRITICAL = "critical",
}

return {
    Stack = Stack,
    Duration = Duration,
    Sync = Sync,
    Coordinator = Coordinator,
    LogLevel = LogLevel,
}
