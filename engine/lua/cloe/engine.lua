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
local fs = require("cloe-engine.fs")
local luax = require("cloe.luax")

local validate = require("cloe.typecheck").validate

--- Let the language-server know we are importing cloe-engine.types into engine:
---@module 'cloe-engine.types'
local engine = {}

-- Import all types from cloe-engine into this namespace.
for k, v in pairs(require("cloe-engine.types")) do
    engine[k] = v
end

--- Return if Cloe has feature as defined by string.
---
--- @param id string feature identifier, such as `cloe-0.20`
--- @return boolean
--- @nodiscard
function engine.has_feature(id)
    validate("cloe.engine.has_feature(string)", id)
    return api.state.features[id] and true or false
end

--- Throw an exception if Cloe does not have feature as defined by string.
---
--- @param id string feature identifier, such as `cloe-0.20`
--- @return nil
function engine.require_feature(id)
    validate("cloe.engine.require_feature(string)", id)
    if not engine.has_feature(id) then
        error("required feature not available: " .. id)
    end
end

--- Try to load (merge) stackfile.
---
--- @param file string File path, possibly relative to calling file
--- @return Stack
function engine.load_stackfile(file)
    validate("cloe.engine.load_stackfile(string)", file)
    local cwd = api.state.current_script_dir or "."
    if fs.is_relative(file) then
        file = cwd .. "/" .. file
    end
    api.state.stack:merge_stackfile(file)
    return api.state.stack
end

--- Read stackfile JSON file as Lua table.
---
--- @param file string File path
--- @return nil
function engine.read_stackfile(file)
    validate("cloe.engine.read_stackfile(string)", file)
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
function engine.apply_stack(stack)
    validate("cloe.engine.apply_stack(string|table)", stack)
    local file = api.state.current_script_file or ""
    if type(stack) == "table" then
        api.state.stack:merge_stacktable(stack, file)
    else
        api.state.stack:merge_stackjson(stack, file)
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
function engine.log(level, fmt, ...)
    validate("cloe.engine.log(string, string, [?any]...)", level, fmt, ...)
    local msg = string.format(fmt, ...)
    api.log(level, "lua", msg)
end

--- Alias a set of signals in the Cloe data broker.
---
--- @param list table
--- @return table
function engine.alias_signals(list)
    api.initial_input.signal_aliases = luax.tbl_extend("force", api.initial_input.signal_aliases, list)
    return api.initial_input.signal_aliases
end

--- Require a set of signals to be made available via the Cloe data broker.
---
--- @param list table signals to merge into main list of required signals
--- @return table # merged list of signals
function engine.require_signals(list)
    api.initial_input.signal_requires = luax.tbl_extend("force", api.initial_input.signal_requires, list)
    return api.initial_input.signal_requires
end

--- Optionally alias and require a set of signals from a signals enum list.
---
--- This allows you to make an enum somewhere which the language server
--- can use for autocompletion and which you can use as an alias:
---
---     ---@enum Sig
---     local Sig = {
---         DriverDoorLatch = "vehicle::framework::chassis::.*driver_door::latch",
---         VehicleMps = "vehicle::sensors::chassis::velocity",
---     }
---     cloe.require_signals_enum(Sig, true)
---
--- Later, you can use the enum with cloe.signal():
---
---     cloe.signal(Sig.DriverDoorLatch)
---
--- @param enum table
--- @param alias boolean whether to treat signal names as alias regular expressions
--- @return nil
function engine.require_signals_enum(enum, alias)
    local signals = {}
    if alias then
        local aliases = {}
        for key, sigregex in pairs(enum) do
            table.insert(aliases, {sigregex, key})
            table.insert(signals, key)
        end
        engine.alias_signals(aliases)
    else
        for _, signame in pairs(enum) do
            table.insert(signals, signame)
        end
    end
    engine.require_signals(signals)
end

--- Return full list of loaded signals.
---
--- @return table
function engine.signals()
    return api.signals
end

--- Return the specified signal.
---
--- @param name string signal name
--- @return any|nil # signal
function engine.signal(name)
    return api.signals[name]
end

--- Set the specified signal with a value.
---
--- @param name string signal name
--- @param value any signal value
--- @return nil
function engine.set_signal(name, value)
    api.signals[name] = value
end

--- Schedule a trigger.
---
--- It is not recommended to use this low-level function, as it is viable to change.
--- Instead, use the engine.schedule, engine.schedule_these, and engine.schedule_test
--- functions.
---
--- @param spec TriggerSchema
--- @return nil
function engine.insert_trigger(spec)
    -- A Lua script runs before a scheduler is started, so the initial
    -- events are put in a queue and picked up by the engine at simulation
    -- start. After this, cloe.state.scheduler exists and we can use its
    -- methods.
    if api.state.scheduler then
        api.state.scheduler:insert_trigger(spec)
    else
        table.insert(api.initial_input.triggers, spec)
    end
end

--- Execute a trigger action directly.
---
--- This is useful when you need to do something but can't wait for
--- a new simulation cycle. Note that not all actions are instantaneous.
---
--- @param action string|table
--- @return nil
function engine.execute_action(action)
    validate("cloe.engine.execute_action(string|table)", action)
    if api.state.scheduler then
        api.state.scheduler:execute_action(action)
    else
        error("can only execute actions within scheduled events")
    end
end

--- @alias ScheduleId integer
--- @alias ScheduleGroup string

--- @class ScheduleSpec
--- @field on string|function|table
--- @field run string|table|fun(sync: Sync):boolean
--- @field desc? string
--- @field enable? boolean|fun():boolean
--- @field group? ScheduleGroup
--- @field pin? boolean
--- @field priority? integer
--- @field source? string

--- @class TestSpec
--- @field id string
--- @field name string
--- @field desc? string
--- @field info? table
--- @field enable? boolean|fun():boolean
--- @field on? string|fun(sync: Sync):boolean
--- @field run fun(z: TestFixture, sync: Sync)
--- @field report? function
--- @field terminate boolean|fun():boolean

--- Return whether the schedule spec is enabled.
---
--- @param spec ScheduleSpec|TestSpec
--- @return boolean
local function is_spec_enabled(spec)
    local default = true
    if spec.enable == nil then
        return default
    elseif type(spec.enable) == "boolean" then
        return spec.enable --[[@as boolean]]
    elseif type(spec.enable) == "function" then
        return spec.enable()
    else
        error("enable: invalid type, expect boolean|fun(): boolean")
    end
end

--- Schedule an event-action pair.
---
--- @param spec ScheduleSpec
--- @return boolean # true if schedule
function engine.schedule(spec)
    luax.validate({ spec = { spec, "table" } })
    luax.validate({
        on = { spec.on, { "string", "table", "function" } },
        run = { spec.run, { "string", "table", "function" } },
        enable = { spec.enable, { "boolean", "function" }, true },
        group = { spec.group, "string", true },
        priority = { spec.priority, "number", true },
        pin = { spec.pin, "boolean", true }, -- sticky
        desc = { spec.desc, "string", true }, -- label
        source = { spec.source, "string", true },
    })
    if not is_spec_enabled(spec) then
        return false
    end

    local event = spec.on
    local action = spec.run
    local action_source = spec.source
    if not action_source and type(action) == "function" then
        local debinfo = debug.getinfo(action)
        action_source = string.format("%s:%s-%s", debinfo.short_src, debinfo.linedefined, debinfo.lastlinedefined)
    end

    -- TODO: Replace this with proper Lua function events
    local pin = spec.pin or false
    if type(event) == "function" then
        local old_event = event
        local old_action = action
        local old_pin = pin
        pin = true
        event = "loop"
        action = function(sync)
            if old_event(sync) then
                if type(old_action) == "function" then
                    old_action(sync)
                else
                    -- TODO: Maybe this works for functions too
                    engine.execute_action(old_action)
                end
                return old_pin
            end
        end
    end

    local group = spec.group or ""
    local priority = spec.priority or 100

    engine.insert_trigger({
        label = spec.desc,
        event = event,
        action = action,
        action_source = action_source,
        sticky = pin,
        priority = priority,
        group = group,
    })
    return true
end

--- Schedule one or more event-action pairs,
--- with defaults specified as keys inline.
---
--- @param specs ScheduleSpec|ScheduleSpec[]
--- @return boolean[]
function engine.schedule_these(specs)
    local results = {}

    luax.validate({ specs = { specs, "table" } })
    luax.validate({
        on = { specs.on, { "string", "table", "function" }, true },
        run = { specs.run, { "string", "table", "function" }, true },
        enable = { specs.enable, { "boolean", "function" }, true },
        group = { specs.group, "string", true },
        priority = { specs.priority, "number", true },
        pin = { specs.pin, "boolean", true }, -- sticky
        desc = { specs.desc, "string", true }, -- label
    })

    for _, trigger in ipairs(specs) do
        local spec = {
            on = trigger.on or specs.on,
            run = trigger.run or specs.run,
            enable = trigger.enable == nil and specs.enable or trigger.enable,
            group = trigger.group or specs.group,
            priority = trigger.priority or specs.priority,
            pin = trigger.pin == nil and specs.pin or trigger.pin,
            desc = trigger.desc or specs.desc,
        }
        local result = engine.schedule(spec)
        table.insert(results, result)
    end

    return results
end

--- Schedule a test as a coroutine that can yield to Cloe.
---
--- @param test TestSpec
function engine.schedule_test(test)
    luax.validate({ test = { test, "table" } })
    luax.validate({
        desc = { test.desc, "string", true },
        enable = { test.enable, { "boolean", "function" }, true },
        id = { test.id, "string" },
        info = { test.info, "table", true },
        name = { test.name, "string", true },
        on = { test.on, { "string", "function" } },
        run = { test.run, "function" },
        terminate = { test.terminate, { "boolean", "function" }, true },
    })
    if not is_spec_enabled(test) then
        return false
    end

    local z = require("cloe.testing").TestFixture.new(test)
    engine.schedule({
        on = test.on,
        group = test.id,
        pin = false,
        desc = test.desc,
        enable = true,
        source = z._source,
        run = function(sync)
            engine.log("info", "Running test: %s", test.id)
            z._sync = sync

            -- Run the actual test
            z:_resume(z, sync)
        end,
    })
end

return engine
