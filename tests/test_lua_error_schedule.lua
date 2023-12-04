-- This example shows that you don't actually need any plugins at
-- all to have a simulation. You can simple schedule some tasks.
local cloe = require("cloe")

cloe.load_stackfile("config_nop_infinite.json")

cloe.schedule {
    on = "loop",
    run = function()
        error("expect error")
    end
}

cloe.schedule {
    desc = "This should not run.",
    on = "time=1",
    run = "succeed",
}
