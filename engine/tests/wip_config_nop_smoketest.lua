local cloe = require("cloe")
cloe.require_version("0.21")

-- Include the basic setup. (This file only
-- schedules test success / fail conditions.)
cloe.load_file("wip_2.lua")

-- It is best practice to check for optional features
-- before using them.
if cloe.has_feature("cloe-server") then
    cloe.setup_server {
        enabled = true
    }
end

cloe.schedule {
    on = "start",
    run = {
        function() cloe.simulation.realtime_factor = -1 end,
        "log=info: Running nop/basic smoketest",
    }
}

-- Multiple actions can be scheduled 
cloe.schedule {
    -- Actions will be assigned the same group ID, which allows
    -- scheduler actions to be performed on the entire set.
    group = "fail_conditions",
    {
        on = "step",
        run = function (_)
            if cloe.vehicles.default["speed_kmph"] > 100 then
                cloe.debug() -- launch interactive debugger
                cloe.simulation.fail("vehicle speed exceeded safe limit")
            end
        end
    },
    {
        on = "virtue.failure",
        run = {
            "log=error: Failed virtue test",
            "fail",
        }
    },
}

cloe.schedule {
    group = "success_conditions",
    on = "time=60s",
    run = "succeed"
}
