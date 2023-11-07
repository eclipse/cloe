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

local types = require("cloe-engine.types")
local validate = require("cloe.typecheck").validate

local actions = {}

--- Stop the simulation.
function actions.stop()
    return "stop"
end

--- Stop the simulation and mark the outcome as failed.
function actions.fail()
    return "fail"
end

--- Stop the simulation and mark the outcome as success.
function actions.succeed()
    return "succeed"
end

--- Insert a trigger at this time.
---
--- @param triggers TriggerConf[]
function actions.insert(triggers)
    return {
        name = "insert",
        items = triggers,
    }
end

--- Keep simulation alive after termination.
---
--- This can be useful if you still want to access the web server.
function actions.keep_alive()
    return "keep_alive"
end

--- Run a command on the system.
---
--- @deprecated Use a Lua function with `cloe.system.exec()`.
--- @param cmd string
--- @param options? { ignore_failure?: boolean, log_output?: string, mode?: string }
function actions.command(cmd, options)
    validate("cloe.actions.command(string)", cmd)
    local trigger = options or {}
    trigger.name = "command"
    trigger.command = cmd
    return trigger
end

--- Log a message with the cloe logging framework.
---
--- @deprecated Use a Lua function with `cloe.log()`.
--- @param level? LogLevel
--- @param msg string
function actions.log(level, msg)
    validate("cloe.actions.log(string?, string)", level, msg)
    return {
        name = "log",
        level = level,
        msg = msg,
    }
end

--- Lua string to execute.
---
--- This is not the recommended way to run Lua as an action.
---
--- @deprecated Use a Lua function directly.
--- @param s string
function actions.lua(s)
    return {
        name = "lua",
        script = s,
    }
end

--- Realtime factor to apply to simulation speed.
---
--- @param factor number where -1 is infinite speed, 0 is invalid, and 1.0 is realtime
function actions.realtime_factor(factor)
    return {
        name = "realtime_factor",
        realtime_factor = factor,
    }
end

return actions
