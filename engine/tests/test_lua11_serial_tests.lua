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
    run = function(z)
        z.printf("Entering TEST-A")
        z.wait_duration("5s")
        z.printf("Waited 5 seconds, complete")
        _G.TRIGGER_NEXT = true
    end
}

cloe.schedule_test {
    id = "TEST-B",
    on = function()
        cloe.log("debug", "Waiting on TRIGGER_NEXT = %s", TRIGGER_NEXT)
        return TRIGGER_NEXT
    end,
    run = function(z, sync)
        z.printf("Entering TEST-B")
        if sync:time() ~= (cloe.Duration.new("5s") + sync:step_width()) then
            z.fail("TEST-B not started at expected time 5s, got %s", sync:time())
        end
        z.succeed()
    end
}

cloe.schedule {
    on = "stop",
    run = function(sync)
        cloe.log("info", "Simulation time is %s", sync:time())
    end
}
