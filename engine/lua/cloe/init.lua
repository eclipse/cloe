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

-- The `cloe` table will be made availabe by cloe-engine, but if it
-- isn't, then use an empty table instead.
--
-- This prevents us from overwriting the fields that cloe-engine sets.
local cloe = assert(cloe)

require("cloe.util")

--- @alias FeatureId string

--- Return if Cloe has feature as defined by string.
---
--- @param id (FeatureId)  feature identifier, such as `cloe-0.20`
--- @return boolean
function cloe.has_feature(id)
    return cloe.state.features[id] and true or false
end

--- Throw an exception if Cloe does not have feature as defined by string.
---
--- @param id (FeatureId)  feature identifier, such as `cloe-0.20`
--- @return nil
function cloe.require_feature(id)
    if not cloe.has_feature(id) then
        cloe.api.experimental.throw_exception("required feature " .. id .. " is not available")
    end
end

--- Try to load stackfile, return deserialized json.
---
--- @param file (string)
--- @return nil
cloe.load_stackfile = function(file)
    assert(cloe.state.stack)
    assert(cloe.api.THIS_SCRIPT_DIR)
    if cloe.fs.is_relative(file) then
        file = cloe.api.THIS_SCRIPT_DIR .. "/" .. file
    end
    cloe.state.stack:merge_stackfile(file)
end

--- Log a message with a given severity.
---
--- @param level (string) severity level, one of: trace, debug, info, warn, error, critical
--- @param fmt (string) format string with trailing arguments compatible with string.format
function cloe.log(level, fmt, ...)
    local msg = string.format(fmt, ...)
    cloe.api.log(level, "lua", msg)
end

--- Schedule
function cloe.schedule(spec)
    cloe.validate({ spec = { spec, "table" }})
    cloe.validate({
        on = { spec.on, {"string", "table"} },
        run = { spec.run, {"string", "table", "function"} },
        priority = { spec.priority, "number", true },
        pin = { spec.pin, "boolean", true },    -- sticky
        desc = { spec.desc, "string", true },   -- label
    })
    local event = spec.on
    local action = spec.run
    local action_source = nil
    if type(action) == "function" then
        local debinfo = debug.getinfo(action)
        action_source = string.format("%s:%s-%s", debinfo.short_src, debinfo.linedefined, debinfo.lastlinedefined)
    end
    local pin =  spec.pin or false
    local priority = spec.priority or 100

    cloe.scheduler.insert({
        label = spec.desc,
        event = event,
        action = action,
        action_source = action_source,
        sticky = pin,
        priority = priority,
    })
end

cloe.scheduler = cloe.scheduler or {}

cloe.state.scheduler_pending = cloe.state.scheduler_pending or {}

function cloe.scheduler.pending()
    return cloe.state.scheduler_pending or {}
end

function cloe.scheduler.insert(spec)
    cloe.state.scheduler_pending = cloe.state.scheduler_pending or {}
    table.insert(cloe.state.scheduler_pending, spec)
end

--- Return a human-readable representation of Lua objects.
---
--- This is primarily used for debugging and should not be used
--- when performance is important. It is a table, but acts as a
--- function.
---
--- For more details, see: https://github.com/kikito/inspect.lua
---
--- @return string representation of object
cloe.inspect = require("cloe.inspect")

--- Wraps cloe.inspect with a print function for faster inspection.
cloe.describe = function(...)
    print(cloe.inspect(...))
end

return cloe
