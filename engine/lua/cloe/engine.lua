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

local typecheck = require("cloe.typecheck")
local validate, validate_shape = typecheck.validate, typecheck.validate_shape

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
    validate("cloe.has_feature(string)", id)
    return api.state.features[id] and true or false
end

--- Throw an exception if Cloe does not have feature as defined by string.
---
--- @param id string feature identifier, such as `cloe-0.20`
--- @return nil
function engine.require_feature(id)
    validate("cloe.require_feature(string)", id)
    if not engine.has_feature(id) then
        error("required feature not available: " .. id)
    end
end

--- Return the active stack configuration as a table.
---
--- Modifying the values here have no effect. It is simply a dump
--- of the JSON representation of a stack configuration.
---
--- @return StackConf
function engine.config()
    return api.state.config
end

--- Try to load (merge) stackfile.
---
--- @param file string file path, possibly relative to calling file
--- @return Stack
function engine.load_stackfile(file)
    validate("cloe.load_stackfile(string)", file)
    local cwd = api.state.current_script_dir or "."
    if fs.is_relative(file) then
        file = cwd .. "/" .. file
    end
    api.state.stack:merge_stackfile(file)
    return api.state.stack
end

--- Read JSON file into Lua types (most likely as Lua table).
---
--- @param file string file path
--- @return any # JSON converted into Lua types
--- @nodiscard
function engine.open_json(file)
    validate("cloe.open_json(string)", file)
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
--- @param stack StackConf|string stack format as Lua table (or JSON string)
--- @return nil
function engine.apply_stack(stack)
    validate("cloe.apply_stack(string|table)", stack)
    local file = api.state.current_script_file or ""
    if type(stack) == "table" then
        api.state.stack:merge_stacktable(stack --[[ @as table ]], file)
    else
        api.state.stack:merge_stackjson(stack --[[ @as string ]], file)
    end
end

--- Log a message with a given severity.
---
--- For example:
---
---     cloe.log("info", "Got value of %d, expected %d", 4, 6)
---     cloe.log(cloe.LogLevel.WARN, "Got value of %s, expected %s", 4, 6)
---
--- @param level LogLevel|string severity level, one of: trace, debug, info, warn, error, critical
--- @param fmt string format string with trailing arguments compatible with string.format
--- @param ... any arguments to format string
--- @return nil
function engine.log(level, fmt, ...)
    validate("cloe.log(string, string, [?any]...)", level, fmt, ...)
    local msg = string.format(fmt, ...)
    api.log(level, "lua", msg)
end

--- Alias a set of signals in the Cloe data broker.
---
--- @param list table<string, string> # regular expression to alias key
--- @return table<string, string> # current signal aliases table
function engine.alias_signals(list)
    -- TODO: Throw an error if simulation already started.
    api.initial_input.signal_aliases = luax.tbl_extend("force", api.initial_input.signal_aliases, list)
    return api.initial_input.signal_aliases
end

--- Require a set of signals to be made available via the Cloe data broker.
---
--- @param list string[] signals to merge into main list of required signals
--- @return string[] # merged list of signals
function engine.require_signals(list)
    -- TODO: Throw an error if simulation already started.
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
--- Later, you can use the enum with `cloe.signal()`:
---
---     cloe.signal(Sig.DriverDoorLatch)
---
--- @param enum table<string, string> input mappging from enum name to signal name
--- @param alias boolean whether to treat signal names as alias regular expressions
--- @return nil
function engine.require_signals_enum(enum, alias)
    -- TODO: Throw an error if simulation already started.
    local signals = {}
    if alias then
        local aliases = {}
        for key, sigregex in pairs(enum) do
            table.insert(aliases, { sigregex, key })
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
--- Example:
---
---     local signals = cloe.signals()
---     signals[SigName] = value
---
--- @return table
function engine.signals()
    return api.signals
end

--- Return the specified signal.
---
--- If the signal does not exist, nil is returned.
---
--- If you want to set the signal, you need to use `cloe.set_signal()`
--- or access the value via `cloe.signals()`.
---
--- @param name string signal name
--- @return any|nil # signal value
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

--- Record the given list of signals into the report.
---
--- This can be called multiple times, but if the signal is already
--- being recorded, then an error will be raised.
---
--- This should be called before simulation starts,
--- so not from a scheduled callback.
---
--- @param list string[] array of signal names
--- @return nil
function engine.record_signals(list)
    validate("cloe.record_signals(string[])", list)
    api.state.report.signals = api.state.report.signals or {}
    local signals = api.state.report.signals
    signals.time = signals.time or {}
    for _, sig in ipairs(list) do
        if signals[sig] then
            error("signal already exists: " .. sig)
        end
        signals[sig] = {}
    end

    cloe.schedule({
        on = "loop",
        pin = true,
        run = function(sync)
            local last_time = signals.time[#signals.time]
            local cur_time = sync:time():ms()
            if last_time ~= cur_time then
                table.insert(signals.time, cur_time)
            end

            for _, sig in ipairs(list) do
                table.insert(signals[sig], cloe.signal(sig))
            end
        end,
    })
end

--- Schedule a trigger.
---
--- It is not recommended to use this low-level function, as it is viable to change.
--- Instead, use one of the following functions:
--- - `cloe.schedule()`
--- - `cloe.schedule_these()`
--- - `cloe.schedule_test()`
---
--- @param trigger TriggerConf
--- @return nil
function engine.insert_trigger(trigger)
    -- A Lua script runs before a scheduler is started, so the initial
    -- events are put in a queue and picked up by the engine at simulation
    -- start. After this, cloe.state.scheduler exists and we can use its
    -- methods.
    if api.state.scheduler then
        api.state.scheduler:insert_trigger(trigger)
    else
        table.insert(api.initial_input.triggers, trigger)
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
    validate("cloe.execute_action(string|table)", action)
    if api.state.scheduler then
        api.state.scheduler:execute_action(action)
    else
        error("can only execute actions within scheduled events")
    end
end

--- @alias EventFunction fun(sync: Sync):boolean

--- @alias ActionFunction fun(sync: Sync):boolean?

--- @class Task
--- @field on string|table|EventFunction what event to trigger on (required)
--- @field run string|table|ActionFunction what to do when the event triggers (required)
--- @field desc? string description of what the trigger is about (default: empty)
--- @field enable? boolean|fun():boolean whether to schedule the trigger or not (default: true)
--- @field group? string whether to assign a group to this trigger (default: nil)
--- @field pin? boolean whether the trigger remains after being run (default: false)
--- @field priority? integer priority to use when multiple events occur simultaneously (currently unimplemented)
--- @field source? string where to the trigger is defined (defined automatically)
local Task
do
    local types = require("tableshape").types
    Task = types.shape {
        on = types.string + types.table + types.func,
        run = types.string + types.table + types.func,
        desc = types.string:is_optional(),
        enable = types.boolean:is_optional(),
        group = types.string:is_optional(),
        pin = types.boolean:is_optional(),
        priority = types.integer:is_optional(),
        source = types.string:is_optional(),
    }
end

--- @class PartialTask
--- @field on? string|table|EventFunction what event to trigger on (required)
--- @field run? string|table|ActionFunction what to do when the event triggers (required)
--- @field desc? string description of what the trigger is about (default: empty)
--- @field enable? boolean|fun():boolean whether to schedule the trigger or not (default: true)
--- @field group? string whether to assign a group to this trigger (default: nil)
--- @field pin? boolean whether the trigger remains after being run (default: false)
--- @field priority? integer priority to use when multiple events occur simultaneously (currently unimplemented)
--- @field source? string where to the trigger is defined (defined automatically)
local PartialTask
local PartialTaskSpec
do
    local types = require("tableshape").types
    PartialTaskSpec = {
        on = (types.string + types.table + types.func):is_optional(),
        run = (types.string + types.table + types.func):is_optional(),
        desc = types.string:is_optional(),
        enable = types.boolean:is_optional(),
        group = types.string:is_optional(),
        pin = types.boolean:is_optional(),
        priority = types.integer:is_optional(),
        source = types.string:is_optional(),
    }
    PartialTask = types.shape(PartialTaskSpec)
end

--- @class Tasks: PartialTask
--- @field [number] PartialTask an array of tasks, falling back to defaults specified above
local Tasks
do
    local types = require("tableshape").types
    Tasks = types.shape(
        PartialTaskSpec,
        {
            extra_fields = types.array_of(PartialTask)
        }
    )
end

--- Expand a list of partial tasks to a list of complete tasks.
---
--- @param tasks Tasks
--- @return Task[]
--- @nodiscard
function engine.expand_tasks(tasks)
    local results = {}
    for _, partial in ipairs(tasks) do
        local task = {
            on = partial.on or tasks.on,
            run = partial.run or tasks.run,
            enable = partial.enable == nil and tasks.enable or partial.enable,
            group = partial.group or tasks.group,
            priority = partial.priority or tasks.priority,
            pin = partial.pin == nil and tasks.pin or partial.pin,
            desc = partial.desc or tasks.desc,
        }
        table.insert(results, task)
    end
    return results
end

--- Return whether the task is enabled.
---
--- @param spec Task|Test
--- @return boolean
--- @nodiscard
local function is_task_enabled(spec)
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

--- Schedule a task (i.e., event-action pair).
---
--- @param task Task
--- @return boolean # true if schedule
function engine.schedule(task)
    validate_shape("cloe.schedule(Task)", Task, task)
    if not is_task_enabled(task) then
        return false
    end

    local event = task.on
    local action = task.run
    local action_source = task.source
    if not action_source and type(action) == "function" then
        local debinfo = debug.getinfo(action)
        action_source = string.format("%s:%s-%s", debinfo.short_src, debinfo.linedefined, debinfo.lastlinedefined)
    end

    -- TODO: Replace this with proper Lua function events
    local pin = task.pin or false
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

    local group = task.group or ""
    local priority = task.priority or 100

    engine.insert_trigger({
        label = task.desc,
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
--- @param tasks Tasks tasks to schedule
--- @return boolean[] # list mapping whether each task was scheduled
function engine.schedule_these(tasks)
    validate_shape("cloe.schedule_these(Tasks)", Tasks, tasks)
    local results = {}
    for _, task in ipairs(engine.expand_tasks(tasks)) do
        local result = engine.schedule(task)
        table.insert(results, result)
    end
    return results
end

--- @class Test
--- @field id string unique identifier to use for test (required)
--- @field on string|EventFunction when to start the test execution (required)
--- @field run fun(z: TestFixture, sync: Sync) test definition (required)
--- @field desc? string description of what the test is about (default: empty)
--- @field info? table metadata to include in the test report (default: nil)
--- @field enable? boolean|fun():boolean whether the test should be scheduled (default: true)
--- @field terminate? boolean|fun():boolean whether to automatically terminate simulation if this is last test run (default: true)
local Test
do
    local types = require("tableshape").types
    Test = types.shape {
        id = types.string,
        on = types.string + types.table + types.func,
        run = types.string + types.table + types.func,
        desc = types.string:is_optional(),
        info = types.table:is_optional(),
        enable = types.boolean:is_optional(),
        terminate = types.boolean:is_optional(),
    }
end

--- Schedule a test as a coroutine that can yield to Cloe.
---
--- @param test Test test specification (requires fields: id, on, run)
function engine.schedule_test(test)
    validate_shape("cloe.schedule_test(Test)", Test, test)
    if not is_task_enabled(test) then
        return false
    end

    --- We don't want users to see private method `schedule_self()`,
    --- but we need to use it here to actually schedule the test.
    --- @diagnostic disable-next-line: invisible
    require("cloe.testing").TestFixture.new(test):schedule_self()
end

return engine
