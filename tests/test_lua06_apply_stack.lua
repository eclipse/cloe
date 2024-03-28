local cloe = require("cloe")

cloe.apply_stack [[{
    "version": "4",
    "include": [
        "config_nop_infinite.json"
    ],
    "server": {
        "listen": false,
        "listen_port": 23456
    }
}]]

-- All the conditions we want to fail on:
cloe.schedule_these {
    run = cloe.actions.fail(),
    { on = "virtue/failure" },
    { on = "default_speed/kmph=>0.0" },
    { on = "time=5" },
}

-- All the things we want to do on start:
cloe.schedule_these {
    on = cloe.events.start(),
    { run = "log=info: Running nop/basic smoketest." },
    { run = "realtime_factor=-1" },
}

cloe.schedule {
    on = cloe.events.loop(),
    pin = true,
    run = function(sync)
        cloe.log(cloe.LogLevel.INFO, "Current time is %s", sync:time())
    end
}

cloe.schedule_test {
    id = "precondition",
    on = cloe.events.start(),
    run = function(z)
        z:printf("hello there")
    end
}

-- Check that schedule_test works as intended.
cloe.schedule_test {
    -- Note that this is the same ID as used in BATS.
    id = "e03fc31f-586b-4e57-80fa-ff2cba5ff9dd",
    on = cloe.events.start(),
    terminate = false,
    run = function(z, sync)
        z:printf("Entering test")
        z:assert(true, "true is truthy")

        z:printf("Asserting something...")
        z:assert_eq(sync:time():s(), 0, "time at start is 0s")

        z:printf("Waiting 1s...")
        z:wait_duration("1s")

        z:assert_ge(sync:time(), cloe.Duration.new("1s"), "yield works and the time has advanced")
        z:assert_eq(sync:time(), cloe.Duration.new("1s"), "time has advanced the right amount")

        z:printf("We're good here.")
        z:succeed()
    end
}
