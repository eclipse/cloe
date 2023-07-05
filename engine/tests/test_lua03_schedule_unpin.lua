-- This example shows that you don't actually need any plugins at
-- all to have a simulation. You can simple schedule some tasks.
local cloe = require("cloe")

cloe.load_stackfile("config_nop_smoketest.json")

cloe.schedule {
    on = "loop",
    priority = 101, -- higher than the default
    pin = false,
    run = function(_)
        cloe.log("info", "Hello world!")
    end
}

cloe.schedule {
    on = "loop",
    pin = true,
    run = function(sync)
        if sync:time():s() % 1 == 0.0 then
            -- Print this every second
            cloe.log("info", "Current time is %s", sync:time())
        end
        if sync:time():s() > 30 then
            -- Unpin
            return false
        end
    end
}
