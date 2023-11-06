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
    run = "fail",
    { on = "virtue/failure" },
    { on = "default_speed/kmph=>0.0" },
    { on = "time=5" },
}

-- All the things we want to do on start:
cloe.schedule_these {
    on = "start",
    { run = "log=info: Running nop/basic smoketest." },
    { run = "realtime_factor=-1" },
}

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
    run = function(z, sync)
        cloe.log("info", "Entering test")
        z:assert(true)

        cloe.log("info", "Asserting something...")
        z:assert(sync:time():s() == 0, "time at start is 0s")

        cloe.log("info", "Waiting 1s...")
        z:wait_duration("1s")

        z:assert(sync:time() >= cloe.Duration.new("1s"), "yield does not work and the time has not advanced")
        z:assert(sync:time() == cloe.Duration.new("1s"), "time has advanced the wrong amount")

        cloe.log("info", "We're good here.")
    end
}
