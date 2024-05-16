-- This example shows that you don't actually need any plugins at
-- all to have a simulation. You can simple schedule some tasks.
local cloe = require("cloe")
local events = cloe.events

cloe.load_stackfile("config_nop_smoketest.json")

cloe.schedule {
    on = events.loop(),
    priority = 101, -- higher than the default
    pin = false,
    run = function(_)
        cloe.log("info", "Hello world!")
    end
}

cloe.schedule {
    on = events.every("1s"),
    pin = true,
    run = function(sync)
        cloe.log("info", "Current time is %s", sync:time())
        if sync:time():s() > 30 then
            -- FIXME: Unpin is not working
            return false
        end
    end
}
