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
--- @field on string
--- @field run function|string|table
--- @field desc? string
--- @field enable? boolean|fun():boolean
--- @field group? ScheduleGroup
--- @field pin? boolean
--- @field priority? integer

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
    cloe.validate({ spec = { spec, "table" }})
    cloe.validate({
        on = { spec.on, {"string", "table"} },
        run = { spec.run, {"string", "table", "function"} },
        enable = { spec.enable, {"boolean", "function"}, true },
        group = { spec.group, "string", true },
        priority = { spec.priority, "number", true },
        pin = { spec.pin, "boolean", true },    -- sticky
        desc = { spec.desc, "string", true },   -- label
    })
    if not is_spec_enabled(spec) then
        return false
    end

    local event = spec.on
    local action = spec.run
    local action_source = nil
    if type(action) == "function" then
        local debinfo = debug.getinfo(action)
        action_source = string.format("%s:%s-%s", debinfo.short_src, debinfo.linedefined, debinfo.lastlinedefined)
    end
    local group = spec.group or ""
    local pin =  spec.pin or false
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
--- @field name string
--- @field desc? string
--- @field enable? boolean|fun():boolean
--- @field on? string -- |string[]|fun():boolean
--- @field run fun(TestFixture):boolean
--- @field report? function

--- @class TestFixture
--- @field id string
--- @field name? string
--- @field desc? string
--- @field assert fun(...)
--- @field print fun(...)
--- @field succeed fun(...)
--- @field fail fun(...)
--- @field abort fun(...)

--- Schedule a test as a coroutine that can yield to Cloe.
---
--- @param test TestSpec
function cloe.schedule_test(test)
    cloe.validate({ test = { test, "table" }})
    cloe.validate({
        desc = { test.desc, "string", true },
        enable = { test.enable, {"boolean", "function"}, true },
        id = { test.id, "string" },
        name = { test.name, "string", true },
        on = { test.on, "string" },
        run = { test.run, "function" },
    })
    if not is_spec_enabled(test) then
        return false
    end

    test._coroutine = coroutine.create(test.run)
    test._resume = function(...)
        local ok, result = coroutine.resume(test._coroutine, ...)
        if not ok then
            error(result)
        elseif type(result) == "table" then
            cloe.schedule(result)
        elseif type(result) == "function" then
            result()
        else
            error("unknown test yield result")
        end
    end

    cloe.schedule({
        on = test.on,
        group = test.id,
        pin = false,
        desc = test.desc,
        enable = true,
        run = function(sync)
            cloe.log("info", "Running test: %s", test.id)
            test._sync = sync
            test._resume(cloe.test_fixture(test), sync)
        end,
    })
end

--- Return a new test fixture for test
function cloe.test_fixture(test)
    local z = {}

    -- Ensure cloe reports has the necessary structure
    cloe.state.report.tests = cloe.state.report.tests or {}
    if cloe.state.report.tests[test.id] then
        error("test already scheduled with id: " .. test.id)
    end
    cloe.state.report.tests[test.id] = {}

    z.report = function(data)
        data = cloe.tbl_extend("error", { time = tostring(test._sync:time()) }, data)
        cloe.log("debug", "Report for %s: %s", test.id, cloe.inspect(data, { indent = ' ', newline = '' }))
        table.insert(cloe.state.report.tests[test.id], data)
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
        coroutine.yield(function()
            coroutine.close(test._coroutine)
        end)
    end

    z.fail = function(fmt, ...)
        fmt = fmt or ""
        z.errorf(fmt, ...)
        z.do_action("fail")
        z.stop()
    end

    z.succeed = function(fmt, ...)
        fmt = fmt or ""
        z.printf(fmt, ...)
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

    return z
end
