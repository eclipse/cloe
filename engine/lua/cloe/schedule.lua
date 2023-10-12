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

local cloe = cloe or {}

--- @alias ScheduleId integer
--- @alias ScheduleGroup string

--- @class ScheduleSpec
--- @field on string|function|table
--- @field run function|string|table
--- @field desc? string
--- @field enable? boolean|fun():boolean
--- @field group? ScheduleGroup
--- @field pin? boolean
--- @field priority? integer
--- @field source? string

local is_spec_enabled = function(spec)
    local default = true
    if spec.enable == nil then
        return default
    elseif type(spec.enable) == "boolean" then
        return spec.enable
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
function cloe.schedule(spec)
    cloe.validate({ spec = { spec, "table" } })
    cloe.validate({
        on = { spec.on, { "string", "table", "function" } },
        run = { spec.run, { "string", "table", "function" } },
        enable = { spec.enable, { "boolean", "function" }, true },
        group = { spec.group, "string", true },
        priority = { spec.priority, "number", true },
        pin = { spec.pin, "boolean", true },  -- sticky
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
                    cloe.scheduler.execute_action(old_action)
                end
                return old_pin
            end
        end
    end

    local group = spec.group or ""
    local priority = spec.priority or 100

    cloe.scheduler.insert({
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
function cloe.schedule_these(specs)
    local results = {}

    cloe.validate({ specs = { specs, "table" } })
    cloe.validate({
        on = { specs.on, { "string", "table", "function" }, true },
        run = { specs.run, { "string", "table", "function" }, true },
        enable = { specs.enable, { "boolean", "function" }, true },
        group = { specs.group, "string", true },
        priority = { specs.priority, "number", true },
        pin = { specs.pin, "boolean", true },  -- sticky
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
        local result = cloe.schedule(spec)
        table.insert(results, result)
    end

    return results
end

cloe.scheduler = cloe.scheduler or {}

cloe.state.scheduler_pending = cloe.state.scheduler_pending or {}

--- Return pending schedules.
--- @return ScheduleSpec[]
function cloe.scheduler.pending()
    return cloe.state.scheduler_pending or {}
end

--- Schedule an event-action pair.
--- @param spec ScheduleSpec
function cloe.scheduler.insert(spec)
    -- A Lua script runs before a scheduler is started, so the initial
    -- events are put in a queue and picked up by the engine at simulation
    -- start. After this, cloe.state.scheduler exists and we can use its
    -- methods.
    if cloe.state.scheduler == nil then
        cloe.state.scheduler_pending = cloe.state.scheduler_pending or {}
        table.insert(cloe.state.scheduler_pending, spec)
    else
        cloe.state.scheduler:insert_trigger(spec)
    end
end

--- Execute a trigger action directly.
---
--- This is useful when you need to do something but can't wait for
--- a new simulation cycle. Note that not all actions are instantaneous.
---
--- @param spec string|table
function cloe.scheduler.execute_action(spec)
    if cloe.state.scheduler == nil then
        error("can only execute actions directly from scheduled events")
    end
    cloe.state.scheduler:execute_action(spec)
end

--- @param match ScheduleId|ScheduleGroup
--- @return table?
function cloe.scheduler.remove(match)
    error("not implemented")
end

--- @class TestSpec
--- @field id string
--- @field info? table
--- @field name string
--- @field desc? string
--- @field enable? boolean|function():boolean
--- @field on? string|function():boolean
--- @field run function(TestFixture):boolean
--- @field report? function

--- @class TestFixture
--- @field id string
--- @field name? string
--- @field desc? string
--- @field assert function(...)
--- @field print function(...)
--- @field succeed function(...)
--- @field fail function(...)
--- @field abort function(...)
--- @field wait_duration function(string)
--- @field wait_until function(function():boolean)
--- @field do_action function(table|string)

--- Schedule a test as a coroutine that can yield to Cloe.
---
--- @param test TestSpec
function cloe.schedule_test(test)
    cloe.validate({ test = { test, "table" } })
    cloe.validate({
        desc = { test.desc, "string", true },
        info = { test.info, "table", true },
        enable = { test.enable, { "boolean", "function" }, true },
        id = { test.id, "string" },
        name = { test.name, "string", true },
        on = { test.on, { "string", "function" } },
        run = { test.run, "function" },
    })
    if not is_spec_enabled(test) then
        return false
    end

    -- Setup report
    if cloe.state.report.tests[test.id] then
        error("test already scheduled with id: " .. test.id)
    end

    cloe.state.report.tests[test.id] = { activity = {}, info = {}, sourceline = "not available" }

    local sourceline = debug.getinfo(2)
    if sourceline then
        cloe.state.report.tests[test.id].sourceline = sourceline.currentline
    end

    -- Insert the info table to the test.id part of the report
    cloe.state.report.tests[test.id].info = test.info

    -- Set up coroutine handling:
    test._coroutine = coroutine.create(test.run)
    test._resume = function(...)
        local ok, result = coroutine.resume(test._coroutine, ...)
        if not ok then
            error(result)
        elseif result then
            local result_type = type(result)
            if result_type == "table" then
                cloe.schedule(result)
            elseif result_type == "function" then
                result()
            else
                error("unknown test yield result: " .. cloe.inspect(result))
            end
        end
    end

    -- In the trigger ensure that the source is the caller of this function
    local debinfo = debug.getinfo(test.run)
    test._source = string.format("%s:%s-%s", debinfo.short_src, debinfo.linedefined, debinfo.lastlinedefined)

    cloe.schedule({
        on = test.on,
        group = test.id,
        pin = false,
        desc = test.desc,
        enable = true,
        source = test._source,
        run = function(sync)
            cloe.log("info", "Running test: %s", test.id)
            test._sync = sync
            test._resume(cloe.test_fixture(test), sync)
        end,
    })
end

local function count_leading_tabs(str)
    local count = 0
    for i = 1, #str do
        local char = string.sub(str, i, i)
        if char == "\t" then
            count = count + 1
        else
            break
        end
    end
    return count
end

--- Return a new test fixture for test
local lust = require("lust")
function cloe.test_fixture(test)
    local z = {}

    z.report = function(data)
        data = cloe.tbl_extend("error", { time = tostring(test._sync:time()) }, data)
        cloe.log("debug", "Report for %s: %s", test.id, cloe.inspect(data, { indent = ' ', newline = '' }))
        table.insert(cloe.state.report.tests[test.id].activity, data)
    end

    z.report_with = function(level, fmt, ...)
        local msg = string.format(fmt, ...)
        cloe.log(level, msg)
        z.report({ message = msg })
    end

    z.debugf = function(fmt, ...) z.report_with("debug", fmt, ...) end
    z.printf = function(fmt, ...) z.report_with("info", fmt, ...) end
    z.warnf = function(fmt, ...) z.report_with("warn", fmt, ...) end
    z.errorf = function(fmt, ...) z.report_with("error", fmt, ...) end

    z.assert = function(expect, fmt, ...)
        cloe.validate({
            expect = { expect, "boolean" },
            fmt = { fmt, "string", true },
        })
        local msg = nil
        if fmt then
            msg = string.format(fmt, ...)
            z.printf("checking condition: %s, %s", expect, msg)
        end
        if not expect then
            z.fail(msg or "assertion failed")
        end
    end

    z.stop = function(fmt, ...)
        if fmt then
            z.printf(fmt, ...)
        end
        coroutine.yield(function()
            coroutine.close(test._coroutine)
        end)
    end

    z.fail = function(fmt, ...)
        if fmt then
            z.errorf(fmt, ...)
        end
        z.do_action("fail")
        z.stop()
    end

    z.succeed = function(fmt, ...)
        if fmt then
            z.printf(fmt, ...)
        end
        z.do_action("succeed")
        z.stop()
    end

    z.wait_duration = function(duration)
        cloe.validate({
            duration = { duration, "string" },
        })
        z.debugf("wait for duration: %s", duration)
        coroutine.yield({
            on = "next=" .. cloe.Duration.new(duration):s(),
            group = test.id,
            run = test._resume
        })
    end

    z.wait_until = function(condition)
        cloe.validate({
            condition = { condition, "function" },
        })
        z.debugf("wait until condition: %s", condition)
        coroutine.yield({
            on = "loop",
            group = test.id,
            pin = true,
            run = function(sync)
                if condition(sync) then
                    test._resume()
                    return false
                end
            end,
        })
    end

    z.do_action = function(action)
        cloe.validate({
            action = { action, { "string", "table" } },
        })
        z.debugf("do action: %s", action)
        cloe.scheduler.execute_action(action)
    end

    z.describe = function(...)
        lust.nocolor()
        local oldprint = _G.print
        local lust_describe_activity = { name = "", evaluation = {} }
        _G.print = function(msg)
            local tab_count = count_leading_tabs(msg)
            msg = cloe.trim(msg) -- remove leading tab
            if tab_count == 0 then
                lust_describe_activity["name"] = msg
            elseif tab_count > 0 then
                table.insert(lust_describe_activity.evaluation, msg)
            end
        end

        lust.describe(...)
        table.insert(cloe.state.report.tests[test.id].activity, lust_describe_activity)
        z.printf("%s", lust_describe_activity)
        _G.print = oldprint
    end

    return z
end
