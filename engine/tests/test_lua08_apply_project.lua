local cloe = require("cloe")
local proj = cloe.require("project")

proj.configure_all {
    with_server = false,
    with_noisy_sensor = true,
}

-- All the conditions we want to fail on:
cloe.schedule_these {
    run = "fail",
    { on = "time=5" },
}

-- All the things we want to do on start:
proj.set_realtime_factor(-1)
cloe.schedule { on = "time=60", run = "succeed" }

cloe.schedule {
    on = "loop",
    pin = true,
    run = function(sync)
        cloe.log("info", "Current time is %s", sync:time())
    end
}

-- Check that schedule_test works as intended.
cloe.schedule_test {
    -- Note that this is the same ID as used in BATS.
    id = "e03fc31f-586b-4e57-80fa-ff2cba5ff9dd",
    on = "start",

    --- @param z TestFixture
    --- @param sync Sync
    run = function(z, sync)
        z:printf("Entering test")
        z:assert(true)

        z:printf("Asserting something...")
        z:assert_eq(sync:time():s(), 0, "time is 0s at start")

        z:printf("Waiting 1s...")
        z:wait_duration("1s")

        z:assert_ge(sync:time(), cloe.Duration.new("1s"), "time has advanced")
        z:assert_eq(sync:time(), cloe.Duration.new("1s"), "time has advanced exactly 1s")

        z:printf("We're good here.")
    end
}
