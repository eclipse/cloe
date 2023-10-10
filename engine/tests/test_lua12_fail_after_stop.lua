local cloe = require("cloe")
local proj = cloe.require("project")

proj.configure_all {
    with_server = false,
    with_noisy_sensor = true,
}

proj.set_realtime_factor(-1)
proj.stop_after("10s")

SHOULD_FAIL = false

-- Check that schedule_test works as intended.
cloe.schedule_test {
    id = "TEST-A",
    on = "time=1",
    run = function(z)
        z.errorf("This has been a bad test!")
        print("bad test")
        SHOULD_FAIL = true
    end
}

cloe.schedule_test {
    id = "TEST-B",
    on = "time=5",
    run = function(z, sync)
        z.printf("TEST-B has been a good test!")
        -- So good it thinks it can say success!
        z.succeed()
    end
}


cloe.schedule {
    on = "stop",
    run = function(sync)
        cloe.log("info", "Checking whether simulation really was successful...")
        cloe.log("info", "- Simulation time is %s", sync:time())
        cloe.log("info", "- SHOULD_FAIL = %s", SHOULD_FAIL)
        if SHOULD_FAIL then
            cloe.scheduler.execute_action("fail")
        end
    end
}
