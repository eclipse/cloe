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

--- Namespace that contains state relevant to a simulation.
cloe.state = cloe.state or {}

require("cloe.util")
require("cloe.schedule")
require("cloe.report")

-- This is just required for documentation.
cloe.fs = require("cloe.fs")

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

--- Try to load (merge) stackfile.
---
--- @param file string File path, possibly relative to calling file
--- @return nil
cloe.load_stackfile = function(file)
    cloe.validate({ stack = { file, "string" } })
    assert(cloe.state.stack)
    assert(cloe.api.THIS_SCRIPT_DIR)
    if cloe.fs.is_relative(file) then
        file = cloe.api.THIS_SCRIPT_DIR .. "/" .. file
    end
    cloe.state.stack:merge_stackfile(file)
end

--- Read stackfile JSON file as Lua table.
---
--- @param file string File path
--- @return nil
cloe.read_stackfile = function(file)
    cloe.validate({ file = { file, "string" } })
    local fp = io.open(file, "r")
    if not fp then
        error("cannot open file: " .. file)
    end
    local data = fp:read("*all")
    local json = require("json")
    return json:decode(data)
end

--- Try to apply the supplied table to the stack.
---
--- @param stack table|string Stack format as Lua table or JSON string
--- @return nil
cloe.apply_stack = function(stack)
    cloe.validate({ stack = { stack, { "string", "table" }} })
    assert(cloe.state.stack)
    local file = cloe.api.THIS_SCRIPT_FILE or ""
    if type(stack) == "table" then
        cloe.state.stack:merge_stacktable(stack, file)
    else
        cloe.state.stack:merge_stackjson(stack, file)
    end
end

--- Log a message with a given severity.
---
--- For example:
---     cloe.log("info", "Got value of %d, expected %d", 4, 6)
---
--- @param level string severity level, one of: trace, debug, info, warn, error, critical
--- @param fmt string format string with trailing arguments compatible with string.format
--- @param ... any arguments to format string
--- @return nil
function cloe.log(level, fmt, ...)
    local msg = string.format(fmt, ...)
    cloe.api.log(level, "lua", msg)
end

--- Return a human-readable representation of Lua objects.
---
--- This is primarily used for debugging and should not be used
--- when performance is important. It is a table, but acts as a
--- function.
---
--- For more details, see: https://github.com/kikito/inspect.lua
---
--- @return string # representation of object
cloe.inspect = require("inspect")

--- Wraps cloe.inspect with a print function for faster inspection.
cloe.describe = function(...)
    print(cloe.inspect(...))
end

return cloe
