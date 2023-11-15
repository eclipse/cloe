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
local validate = require("cloe.typecheck").validate

local events = {}

--- Event to be used for the on key in schedule_test.
---
--- Example:
---
---     cloe.schedule_test {
---         id = "TEST-A",
---         on = cloe.events.start(),
---         -- ...
---     }
---
---     cloe.schedule_test {
---         id = "TEST-B",
---         on = cloe.events.after_tests("TEST-A"),
---         -- ...
---     }
---
---     cloe.schedule_test {
---         id = "FINAL",
---         on = cloe.events.after_tests("TEST-A", "TEST-B"),
---         -- ...
---     }
---
--- @param ... string tests to wait for
--- @return fun():boolean
function events.after_tests(...)
    validate("cloe.events.after_tests(string...)", ...)
    local names = { ... }

    if #names == 1 then
        local name = names[1]
        return function()
            return api.state.report.tests[name].complete
        end
    else
        return function()
            for _, k in ipairs(names) do
                if not api.state.report.tests[k].complete then
                    return false
                end
            end
            return true
        end
    end
end

--- Schedule every duration, starting with 0.
---
--- Note: You have to pin the schedule otherwise it will be descheduled
--- after running once.
---
--- @param duration string|Duration
function events.every(duration)
    validate("cloe.events.every(string|userdata)", duration)
    if type(duration) == "string" then
        duration = types.Duration.new(duration)
    end
    if duration:ns() % api.state.config.simulation.model_step_width ~= 0 then
        error("interval duration is not a multiple of nominal step width")
    end
    return function(sync)
        return sync:time():ms() % duration:ms() == 0
    end
end

--- When the simulation is starting.
function events.start()
    return "start"
end

--- When the simulation has stopped.
function events.stop()
    return "stop"
end

--- When the simulation is marked as a fail (after stopping).
function events.failure()
    return "failure"
end

--- When the simulation is marked as a pass (after stopping).
function events.success()
    return "success"
end

--- Every loop.
---
--- Note: You have to pin the schedule otherwise it will be descheduled
--- after running once.
function events.loop()
    return "loop"
end

--- Schedule for absolute simulation time specified.
---
--- Warning: If the specified time is in the past, then the behavior is *undefined*.
---
--- @param simulation_time string|Duration
function events.time(simulation_time)
    validate("cloe.events.every(string|userdata)", simulation_time)
    if type(simulation_time) == "string" then
        simulation_time = types.Duration.new(simulation_time)
    end
    return string.format("time=%s", simulation_time:s())
end

--- Schedule for next cycle after specified duration.
---
--- @param simulation_duration? string|Duration
function events.next(simulation_duration)
    validate("cloe.events.next([string|userdata])", simulation_duration)
    if not simulation_duration then
        return "next"
    end

    if type(simulation_duration) == "string" then
        simulation_duration = types.Duration.new(simulation_duration)
    end
    return string.format("next=%s", simulation_duration:s())
end

--- When the simulation is paused.
---
--- This will trigger every few milliseconds while in the pause state.
function events.pause()
    return "pause"
end

--- When the simulation resumes after pausing.
function events.resume()
    return "resume"
end

--- When the simulation is reset.
---
--- @deprecated Currently this behavior is unsupported.
function events.reset()
    return "reset"
end

--- When condition() is true or after timeout duration has elapsed.
---
--- NOTE: Currently it is not possible to easily determine if the
--- event is triggering because of a timeout or because the condition
--- evaluated to true.
---
--- Example:
---
---     cloe.schedule {
---       on = cloe.events.with_timeout(nil, function(sync) return cloe.signal("SIG_A") == 5 end, "10s"),
---       action = cloe.actions.succeed(),
---     }
---
--- @param current_sync? Sync current Sync, possibly nil
--- @param condition fun(sync: Sync):boolean
--- @param timeout string|Duration time to wait until giving up
function events.with_timeout(current_sync, condition, timeout)
    if type(timeout) == "string" then
        timeout = types.Duration.new(timeout)
    end
    if current_sync then
        timeout = current_sync:time() + timeout
    end
    return function(sync)
        if sync:time() > timeout then
            return true
        end
        return condition(sync)
    end
end

return events
