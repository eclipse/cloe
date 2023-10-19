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
--- @meta cloe-engine
---
--- This file contains the type annotations of the `cloe-engine` module,
--- which are exported by the cloe-engine executable.
---
--- These methods should only be used by the cloe library.
---

local engine = {
    -- Contains data that will be processed at simulation start,
    -- but will not be considered afterward.
    initial_input = {
        triggers = {},
        triggers_processed = 0,
        signal_aliases = {},
        signal_requires = {},
    },

    -- Contains engine state for a simulation.
    state = {
        features = {},
        report = {},
        scheduler = nil,
        stack = nil,
        current_script_file = nil,
        current_script_dir = nil,
        scripts_loaded = {},
    },

    plugins = {},

    signals = {},
}

require("cloe-engine.types")

--- Fail with an error message that cloe-engine functionality not available.
---
--- @param fname string
--- @param ... any Consumed but not used
--- @return any
local function unavailable(fname, ...)
    local inspect = require("inspect").inspect
    local buf = "cloe-engine." .. fname .. "("
    for i, v in ipairs(...) do
        if i ~= 1 then
            buf = buf .. ", "
        end
        buf = buf .. inspect(v)
    end
    buf = buf .. ")"
    error(string.format("error: %s: implementation unavailable outside cloe-engine", buf))
end

--- Return two-character string representation of log-level.
---
--- @param level string
--- @return string
--- @nodiscard
local function log_level_format(level)
    if level == "info" then
        return "II"
    elseif level == "debug" then
        return "DD"
    elseif level == "warn" then
        return "WW"
    elseif level == "error" then
        return "EE"
    elseif level == "critical" then
        return "CC"
    elseif level == "trace" then
        return "TT"
    else
        return "??"
    end
end

--- Return whether the engine is available.
---
--- This is not the case when a Lua script is being run with
--- another interpreter, a REPL, or a language server.
---
--- @return boolean
function engine.is_available()
    return false
end

--- Return path to Lua file that the engine is currently merging,
--- or nil if no file is being loaded.
---
--- @return string|nil
function engine.get_script_file()
    return engine.state.current_script_file
end

--- Return path to directory containing the Lua file that the engine is
--- currently merging, or nil if no file is being loaded.
---
--- @return string|nil
function engine.get_script_dir()
    return engine.state.current_script_dir
end

--- Return the global Stack instance.
---
--- @return Stack
function engine.get_stack()
    return unavailable("get_stack")
end

--- Return the simulation scheduler (aka. Coordinator) global instance.
---
--- @return Coordinator
function engine.get_scheduler()
    return unavailable("get_scheduler")
end

--- Return the simulation report.
---
--- @return table
function engine.get_report()
    return engine.state.report
end

--- Return a table of available features.
---
--- @return table
function engine.get_features()
    return engine.state.features
end

--- Log a message.
---
--- @param level string
--- @param prefix string
--- @param message string
--- @return nil
function engine.log(level, prefix, message)
    print(string.format("%s %s [%s] %s", log_level_format(level), os.date("%T"), prefix, message))
end

--- @class CommandSpecA
--- @field path string name or path of executable
--- @field args table list of arguments
--- @field mode? string execution mode (one of "sync", "async", "detach")
--- @field log_output? string output verbosity ("never", "on_error", "always")
--- @field ignore_failure? boolean whether to ignore failure

--- @class CommandSpecB
--- @field command string command or script to run with default shell
--- @field mode? string execution mode (one of "sync", "async", "detach")
--- @field log_output? string output verbosity ("never", "on_error", "always")
--- @field ignore_failure? boolean whether to ignore failure

--- @alias CommandSpecC string command or script to run with default shell

--- @alias CommandSpec (CommandSpecA | CommandSpecB | CommandSpecC)

--- Run a system command with the cloe executer.
---
--- @param spec CommandSpec
--- @return string,number
function engine.exec(spec)
    return unavailable("exec", spec), 1
end

return engine
