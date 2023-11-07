local cloe = require("cloe")
local proj = cloe.require("project")

proj.configure_all {
    with_server = false,
    with_noisy_sensor = true,
}

proj.set_realtime_factor(-1)

-- Check that schedule_test works as intended.
cloe.schedule_test {
    id = "TEST-A",
    on = cloe.events.time("1s"),
    run = function(z)
        z:expect(false, "This has been a bad test!")
    end
}

cloe.schedule_test {
    id = "TEST-B",
    on = cloe.events.time("5s"),
    run = function(z)
        z:expect(true, "TEST-B has been a good test!")
        z:assert
    end
}

cloe.schedule {
    on = "stop",
    run = function(sync)
        cloe.log("info", "Checking whether simulation really was successful...")
        cloe.log("info", "- Simulation time is %s", sync:time())
    end
}
