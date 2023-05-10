-- This example shows that you don't actually need any plugins at
-- all to have a simulation. You can simple schedule some tasks.
local cloe = require("cloe")

cloe.setup_scheduler {
    step_size = "100ms",
}

cloe.schedule {
    on = "step",
    priority = 101, -- higher than the default
    pin = false,
    run = function(_)
        cloe.log("info", "Hello world!")
    end
}

cloe.schedule {
    on = "step",
    pin = true,
    run = function(ctx)
        cloe.log("info", "Current time is %s", ctx.simulation_time())
    end
}

cloe.schedule {
    on = "time=1s",
    run = "succeed"
}
