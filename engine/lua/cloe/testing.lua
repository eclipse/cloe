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
local types = require("cloe-engine.types")

local luax = require("cloe.luax")
local validate = require("cloe.typecheck").validate
local inspect = require("inspect").inspect

--- @class TestFixture
--- @field private _test table
--- @field private _id string
--- @field private _report table
--- @field private _coroutine thread
--- @field private _source string
--- @field private _sync Sync
--- @field private _asserts integer
--- @field private _failures integer
local TestFixture = {}

--- @class SchedulerInterface
---
--- Interface to scheduler to support dependency injection and
--- so we can avoid cyclic dependency to/from cloe.engine.
---
--- @field log fun(level: string, fmt: string, ...: any)
--- @field schedule fun(trigger: ScheduleSpec)
--- @field execute_action fun(action: string|table)

--- @enum TestStatus
local TestStatus = {
    PENDING = "pending", --- Waiting to be scheduled
    RUNNING = "running", --- Currently running
    ABORTED = "aborted", --- Aborted because of error
    STOPPED = "stopped", --- Stopped without explicit pass/fail
    FAILED = "failed", --- Stopped with >=1 asserts failed
    PASSED = "passed", --- Stopped with all asserts passed
}

--- @enum ReportOutcome
local ReportOutcome = {
    FAILURE = "fail", --- >=1 tests failed
    SUCCESS = "pass", --- all tests passed
}

--- Return a new test fixture for test
---
--- @param test TestSpec
--- @param scheduler? SchedulerInterface for dependency injection
--- @return TestFixture
function TestFixture.new(test, scheduler)
    scheduler = scheduler or {}

    local debinfo = debug.getinfo(test.run)
    local source = string.format("%s:%s-%s", debinfo.short_src, debinfo.linedefined, debinfo.lastlinedefined)

    local report = api.state.report
    if report["tests"] == nil then
        report["tests"] = {}
    end
    if report.tests[test.id] then
        error("test already scheduled with id: " .. test.id)
    else
        report.tests[test.id] = {
            complete = false,
            status = TestStatus.PENDING,
            info = test.info,
            activity = {},
            source = source,
        }
    end

    return setmetatable({
        _id = test.id,
        _test = test,
        _report = report.tests[test.id],
        _coroutine = coroutine.create(test.run),
        _source = source,
        _stopped = false,
        _asserts = 0,
        _asserts_failed = 0,
        _asserts_passed = 0,
        _log = scheduler.log,
        _schedule = scheduler.schedule,
        _execute_action = scheduler.execute_action,
    }, {
        __index = TestFixture,
    })
end

--- Log a message.
---
--- @private
--- @param level string
--- @param message string
--- @param ... any
--- @return nil
function TestFixture:_log(level, message, ...)
    require("cloe.engine").log(level, message, ...)
end

--- Schedule resumption of test.
---
--- @private
--- @param spec ScheduleSpec
function TestFixture:_schedule(spec)
    require("cloe.engine").schedule(spec)
end

--- Execute an action immediately.
---
--- @private
--- @param action string|table
function TestFixture:_execute_action(action)
    require("cloe.engine").execute_action(action)
end

--- Resume execution of the test after an interuption.
---
--- This is called at the beginning of the test, and any time it
--- hands control back to the engine to do other work.
---
--- @private
--- @param ... any
--- @return nil
function TestFixture:_resume(...)
    self:_log("debug", "Resuming test %s", self._id)
    self:_set_status(TestStatus.RUNNING)
    local ok, result = coroutine.resume(self._coroutine, ...)
    if not ok then
        self:_set_status(TestStatus.ABORTED)
        error(string.format("Error with test %s: %s", self._id, result))
    elseif result then
        local result_type = type(result)
        if result_type == "table" then
            -- From self:wait*() methods
            self:_schedule(result)
            self:_set_status(TestStatus.PENDING)
        elseif result_type == "function" then
            -- From self:stop() methods
            result()
            self:_finish()
        else
            self:_set_status(TestStatus.ABORTED)
            error("unknown test yield result: " .. inspect(result))
        end
    else
        -- From end-of-test-case
        self:_finish()
    end
end

--- @private
function TestFixture:_finish()
    -- After the test completes, update the report
    self._report["asserts"] = {
        total = self._asserts,
        failed = self._asserts_failed,
        passed = self._asserts_passed,
    }
    if self._asserts_failed > 0 then
        self:_set_status(TestStatus.FAILED)
    else
        self:_set_status(TestStatus.PASSED)
    end
    self._report.complete = true
    self:_terminate()
end

--- @private
--- @param status TestStatus
function TestFixture:_set_status(status)
    self:_log("debug", "[%s] Status -> %s", self._id, status)
    self._report.status = status
end

--- @private
function TestFixture:_terminate()
    local report = api.state.report
    local tests = 0
    local tests_failed = 0
    for _, test in pairs(report["tests"]) do
        if not test.complete then
            -- Not all tests complete, let the next fixture do the job
            return
        end

        tests = tests + 1
        if test.status ~= TestStatus.PASSED then
            tests_failed = tests_failed + 1
        end
    end
    if tests_failed ~= 0 then
        report.outcome = ReportOutcome.FAILURE
    else
        report.outcome = ReportOutcome.SUCCESS
    end

    local term = self._test.terminate == nil and true or self._test.terminate
    if type(term) == "function" then
        term = term(self, self._sync)
    end
    if term then
        self:_log("info", "Terminating simulation (disable with terminate=false)...")
        if report.outcome == ReportOutcome.FAILURE then
            self:_execute_action("fail")
        elseif report.outcome == ReportOutcome.SUCCESS then
            self:_execute_action("succeed")
        else
            self:_execute_action("stop")
        end
    end
end

--- Add some data to the report.
---
--- Will also log the message as debug.
---
--- @param data table
--- @param quiet? boolean
--- @param level? string
--- @return nil
function TestFixture:report_data(data, quiet, level)
    data = luax.tbl_extend("error", { time = tostring(self._sync:time()) }, data)
    if not quiet then
        self:_log(level or "debug", "[%s] Report: %s", self._id, inspect(data, { indent = " ", newline = "" }))
    end
    table.insert(self._report.activity, data)
end

--- Add a message to the field to the report and log it with the given severity.
---
--- @param field string Field to assign message to.
--- @param level string Severity to log the message.
--- @param fmt string Format string.
--- @param ... any Arguments to format string.
--- @return nil
function TestFixture:report_with(field, level, fmt, ...)
    validate("TestFixture:report_with(string, string, string, [?any]...)", self, field, level, fmt, ...)
    local msg = string.format(fmt, ...)
    self:_log(level, "[%s] Report %s: %s", self._id, field, msg)
    self:report_data({ [field] = msg }, true)
end

--- Add a message to the report and log it with the given severity.
---
--- @param level string Severity to log the message.
--- @param fmt string Format string.
--- @param ... any Arguments to format string.
--- @return nil
function TestFixture:report_message(level, fmt, ...)
    validate("TestFixture:report_message(string, string, [?any]...)", self, level, fmt, ...)
    self:report_with("message", level, fmt, ...)
end

--- Log a message to report and console in debug severity.
---
--- @param fmt string
--- @param ... any
--- @return nil
function TestFixture:debugf(fmt, ...)
    self:report_message("debug", fmt, ...)
end

--- Log a message to report and console in info severity.
---
--- @param fmt string
--- @param ... any
--- @return nil
function TestFixture:printf(fmt, ...)
    self:report_message("info", fmt, ...)
end

--- Log a message to report and console in warn severity.
---
--- @param fmt string
--- @param ... any
--- @return nil
function TestFixture:warnf(fmt, ...)
    self:report_message("warn", fmt, ...)
end

--- Log a message to report and console in error severity.
---
--- Note: this does not have an effect on the test results.
---
--- @param fmt string
--- @param ... any
--- @return nil
function TestFixture:errorf(fmt, ...)
    self:report_message("error", fmt, ...)
end

--- Terminate the execution of the test-case, but not the simulation.
---
--- @param fmt? string
--- @param ... any
--- @return nil
function TestFixture:stop(fmt, ...)
    validate("TestFixture:stop([string], [?any]...)", self, fmt, ...)
    if fmt then
        self:printf(fmt, ...)
    end
    coroutine.yield(function()
        if self._report.status == TestStatus.PENDING then
            self:_set_status(TestStatus.STOPPED)
        end
        end
        coroutine.close(self._coroutine)
    end)
end

--- Fail the test-case and stop the simulation.
---
--- Note: It is best practice to use expect and assert methods and allow the
--- test-case fixture to determine failure/success itself.
---
--- @param fmt string optional message
--- @param ... any
--- @return nil
function TestFixture:fail(fmt, ...)
    validate("TestFixture:fail([string], [?any]...)", self, fmt, ...)
    if fmt then
        self:errorf(fmt, ...)
    end
    self:do_action("fail")
    self:stop()
end

--- Succeed the test-case and stop the simulation.
---
--- Note: It is best practice to use expect and assert methods and allow the
--- test-case fixture to determine failure/success itself.
---
--- @param fmt? string optional message
--- @param ... any
--- @return nil
function TestFixture:succeed(fmt, ...)
    validate("TestFixture:succeed([string], [?any]...)", self, fmt, ...)
    if fmt then
        self:printf(fmt, ...)
    end
    self:do_action("succeed")
    self:stop()
end

--- Wait simulated time given in duration units, e.g. "1.5s".
---
--- This will yield execution of the test-case back to the simulation
--- until the duration has elapsed.
---
--- @param duration string
--- @return nil
function TestFixture:wait_duration(duration)
    validate("TestFixture:wait_duration(string)", self, duration)
    self:debugf("wait for duration: %s", duration)
    coroutine.yield({
        on = "next=" .. types.Duration.new(duration):s(),
        group = self._id,
        run = function(sync)
            return self:_resume()
        end,
    })
end

--- Wait until the function supplied returns true, then resume.
---
--- This will yield execution of the test-case back to the simulation
--- until the function, which is run once every cycle, returns true.
---
--- @param condition fun(sync: Sync):boolean
--- @param timeout? Duration|string
--- @return nil
function TestFixture:wait_until(condition, timeout)
    validate("TestFixture:wait_until(function, [string|userdata])", self, condition, timeout)
    if type(timeout) == "string" then
        timeout = types.Duration.new(timeout)
    end
    if timeout then
        timeout = self._sync:time() + timeout
        self:debugf("wait until condition with timeout %s: %s", timeout, condition)
    else
        self:debugf("wait until condition: %s", condition)
    end
    coroutine.yield({
        on = "loop",
        group = self._id,
        pin = true,
        run = function(sync)
            if condition(sync) then
                self:_resume(true)
                return false
            elseif timeout and sync:time() > timeout then
                self:warnf("condition timed out after %s", timeout)
                self:_resume(false)
                return false
            end
        end,
    })
end

function TestFixture:do_action(action)
    validate("TestFixture:do_action(string|table)", self, action)
    self:report_with("action", "debug", "%s", action)
    self:_execute_action(action)
end

--- @class Operator
--- @field fn fun(left: any, right: any):boolean
--- @field repr string

--- Expect an operation with op(left, right) == true.
---
--- @private
--- @param op Operator
--- @return boolean # result of expression
function TestFixture:_expect_op(op, left, right, fmt, ...)
    validate("TestFixture:_expect_op(table, any, any, [string], [?any]...)", self, op, left, right, fmt, ...)
    self._asserts = self._asserts + 1
    local msg = nil
    if fmt then
        msg = string.format(fmt, ...)
    end
    local report = {
        assert = string.format("%s %s %s", left, op.repr, right),
        left = inspect(left, { newline = " ", indent = "" }),
        right = inspect(right, { newline = " ", indent = "" }),
        value = op.fn(left, right),
        message = msg,
    }
    self:report_data(report, true)
    if report.value then
        self._asserts_passed = self._asserts_passed + 1
        self:_log("info", "[%s] Check %s: %s (=%s)", self._id, msg or "ok", report.assert, report.value)
    else
        self._asserts_failed = self._asserts_failed + 1
        self:_log("error", "[%s] !! Check %s: %s (=%s)", self._id, msg or "failed", report.assert, report.value)
    end
    return report.value
end

--- Assert that the first argument is truthy.
---
--- The message should describe the expectation:
---
---     z.assert(var == 1, "var should == 1")
---
--- You should check if a more specific assertion is available first though,
--- as these provide better messages.
---
--- @param value any
--- @param fmt? string
--- @param ... any
--- @return any value
function TestFixture:expect(value, fmt, ...)
    return self:_expect_op({
        fn = function(a)
            return a
        end,
        repr = "is",
    }, value, "truthy", fmt, ...)
end

--- Assert that the first argument is true.
---
--- The message should describe the expectation:
---
---     z.assert(var == 1, "var should == 1")
---
--- You should check if a more specific assertion is available first though,
--- as these provide better messages.
---
--- @param value any
--- @param fmt? string
--- @param ... any
--- @return nil
function TestFixture:assert(value, fmt, ...)
    if not self:expect(value, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

local Operator = {
    eq = { fn = function(a, b) return a == b end, repr = "==", }, --- Equal to
    ne = { fn = function(a, b) return a ~= b end, repr = "~=", }, --- Not equal to
    lt = { fn = function(a, b) return a < b end, repr = "<", },   --- Less than
    le = { fn = function(a, b) return a <= b end, repr = "<=", }, --- Less than or equal to
    gt = { fn = function(a, b) return a > b end, repr = ">", },   --- Greater than
    ge = { fn = function(a, b) return a >= b end, repr = ">=", }, --- Greater than or equal to
}

function TestFixture:expect_eq(left, right, fmt, ...)
    return self:_expect_op(Operator.eq, left, right, fmt, ...)
end

function TestFixture:expect_ne(left, right, fmt, ...)
    return self:_expect_op(Operator.ne, left, right, fmt, ...)
end

function TestFixture:expect_lt(left, right, fmt, ...)
    return self:_expect_op(Operator.lt, left, right, fmt, ...)
end

function TestFixture:expect_le(left, right, fmt, ...)
    return self:_expect_op(Operator.le, left, right, fmt, ...)
end

function TestFixture:expect_gt(left, right, fmt, ...)
    return self:_expect_op(Operator.gt, left, right, fmt, ...)
end

function TestFixture:expect_ge(left, right, fmt, ...)
    return self:_expect_op(Operator.ge, left, right, fmt, ...)
end

function TestFixture:assert_eq(left, right, fmt, ...)
    if not self:_expect_op(Operator.eq, left, right, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

function TestFixture:assert_ne(left, right, fmt, ...)
    if not self:_expect_op(Operator.ne, left, right, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

function TestFixture:assert_lt(left, right, fmt, ...)
    if not self:_expect_op(Operator.lt, left, right, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

function TestFixture:assert_le(left, right, fmt, ...)
    if not self:_expect_op(Operator.le, left, right, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

function TestFixture:assert_gt(left, right, fmt, ...)
    if not self:_expect_op(Operator.gt, left, right, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

function TestFixture:assert_ge(left, right, fmt, ...)
    if not self:_expect_op(Operator.ge, left, right, fmt, ...) then
        self:fail("[%s] test assertion failed", self._id)
    end
end

--- Return number of leading tabs in string.
---
--- @param str string
--- @return number
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

--- Start a description block based on the Lust framework.
---
--- The Lust framework provides Behavior-Driven-Development (BDD) style
--- test tooling. See their website for more information:
---
---     https://github.com/bjornbytes/lust
---
--- Warning: In unfortunate circumstances, using this method may (in its
--- current implementation) result in error messages and/or other output
--- from Lua that uses the print() statement being suppressed.
---
--- @deprecated EXPERIMENTAL
--- @param name string Description of subject
--- @param fn fun() Function to scope test execution
--- @return nil
function TestFixture:describe(name, fn)
    local lust = require("lust")

    -- Lust uses print(), so we hijack the function temporarily to capture
    -- its output.
    --
    -- NOTE: This also means that if there is some kind of error within
    -- such a describe block, we may not re-install the original print
    -- function and all further output may be suppressed!
    local oldprint = _G.print
    local lust_describe_activity = { name = "", evaluation = {} }
    _G.print = function(msg)
        local tab_count = count_leading_tabs(msg)
        msg = luax.trim(msg) -- remove leading tab
        if tab_count == 0 then
            lust_describe_activity["name"] = msg
        elseif tab_count > 0 then
            table.insert(lust_describe_activity.evaluation, msg)
        end
    end

    lust.nocolor()
    lust.describe(name, fn)
    self:report_data(lust_describe_activity)
    _G.print = oldprint
end

return {
    TestFixture = TestFixture,
}
