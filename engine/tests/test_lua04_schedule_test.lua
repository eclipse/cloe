local cloe = require("cloe")

cloe.load_stackfile("config_nop_smoketest.json")

-- If schedule_test does not work, then we will keep running until
-- this event triggers and we fail.
cloe.schedule {
    on = "time=5",
    run = "fail",
}

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
    run = function(z, sync)
        cloe.log("info", "Entering test")
        z.assert(true)

        cloe.log("info", "Asserting something...")
        z.assert(sync:time():s() == 0, "time at start is 0s")

        cloe.log("info", "Waiting 1s...")
        z.wait_duration("1s")

        z.assert(sync:time() >= cloe.Duration.new("1s"), "yield does not work and the time has not advanced")
        z.assert(sync:time() == cloe.Duration.new("1s"), "time has advanced the wrong amount")

        cloe.log("info", "We're good here.")
        z.succeed()
    end
}
