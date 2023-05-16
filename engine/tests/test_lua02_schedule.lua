-- This example shows that you don't actually need any plugins at
-- 
-- all to have a simulation. You can simple schedule some tasks.
local cloe = require("cloe")

cloe.log("info", "Hello world!");

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
        cloe.log("info", "Current time is %s", sync:time())
    end
}

cloe.schedule {
    on = "time=1",
    run = "succeed"
}
