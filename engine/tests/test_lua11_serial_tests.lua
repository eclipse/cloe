local cloe = require("cloe")
local proj = cloe.require("project")

proj.configure_all {
    with_server = false,
    with_noisy_sensor = true,
}

proj.set_realtime_factor(-1)
proj.fail_after("10s")

TRIGGER_NEXT = false

-- Check that schedule_test works as intended.
cloe.schedule_test {
    id = "TEST-A",
    on = "start",

    --- @param z TestFixture
    run = function(z)
        z:printf("Entering TEST-A")
        z:wait_duration("5s")
        z:printf("Waited 5 seconds, complete")
        _G.TRIGGER_NEXT = true
    end
}

local dur = cloe.Duration.new

cloe.schedule_test {
    id = "TEST-B",
    on = function()
        cloe.log("debug", "Waiting on TRIGGER_NEXT = %s", TRIGGER_NEXT)
        return TRIGGER_NEXT
    end,

    --- @param z TestFixture
    --- @param sync Sync
    run = function(z, sync)
        z:printf("Entering TEST-B")
        z:assert(sync:time() == (dur("5s") + sync:step_width()), "TEST-B should start after TEST-A completed, at 5s")
    end
}

cloe.schedule {
    on = "stop",
    run = function(sync)
        cloe.log("info", "Simulation time is %s", sync:time())
    end
}
