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

local api = require("cloe-engine")
local engine = require("cloe.engine")

---@module 'cloe.engine'
local cloe = {}

-- Re-export everything from cloe.engine into cloe.
for k, v in pairs(engine) do
    if cloe[k] then
        error("duplicate function definition in cloe.engine")
    end
    cloe[k] = v
end

--- Table of functions for dealing with file paths.
cloe.fs = require("cloe-engine.fs")

--- Table of common events for use with tasks and tests.
cloe.events = require("cloe.events")

--- Table of common actions for use with tasks.
cloe.actions = require("cloe.actions")

--- Validate input arguments of a function in a single line.
---
--- This is basically a specialized version of the typecheck.argscheck
--- function, in that it does not wrap the original function,
--- thereby preserving the type data that the Lua language server
--- uses to provide hints and autocompletion.
---
--- @see cloe.typecheck.validate
cloe.validate = require("cloe.typecheck").validate

--- Validate the shape (from tableshape) of a table or type.
---
--- @see cloe.typecheck.validate_shape
cloe.validate_shape = require("cloe.typecheck").validate_shape

--- Return a human-readable representation of a Lua object.
---
--- This is primarily used for debugging and should not be used
--- when performance is important. It is a table, but acts as a
--- function.
---
--- For more details, see: https://github.com/kikito/inspect.lua
---
--- @see inspect.inspect
cloe.inspect = require("inspect").inspect

--- Print a human-readable representation of a Lua object.
---
--- This just prints the output of inspect.
---
--- For more details, see: https://github.com/kikito/inspect.lua
---
--- @param root any
--- @param options? table
--- @return nil
function cloe.describe(root, options)
    print(require("inspect").inspect(root, options))
end

--- Require a module, prioritizing modules relative to the script
--- launched by cloe-engine.
---
--- If api.get_script_dir() is nil, this is equivalent to require().
---
--- @param module string module identifier, such as "project"
function cloe.require(module)
    cloe.validate("cloe.require(string)", module)
    local script_dir = api.get_script_dir()
    if script_dir then
        local old_package_path = package.path
        package.path = string.format("%s/?.lua;%s/?/init.lua;%s", script_dir, script_dir, package.path)
        local module_table = require(module)
        package.path = old_package_path
        return module_table
    else
        engine.log("warn", "cloe.require() expects cloe-engine.get_script_dir() ~= nil, but it is not", nil)
        return require(module)
    end
end

--- Initialize report metadata.
---
--- @param header table Optional report header information that will be merged in.
--- @return table
function cloe.init_report(header)
    cloe.validate("cloe.init_report(?table)", header)
    local system = require("cloe.system")
    local report = api.get_report()
    report.metadata = {
        hostname = system.get_hostname(),
        username = system.get_username(),
        datetime = system.get_datetime(),
    }
    if header then
        report.metadata = require("cloe.luax").tbl_deep_extend("force", report.metadata, header)
    end
    return report
end

return cloe
