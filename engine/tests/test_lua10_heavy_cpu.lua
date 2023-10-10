local cloe = require("cloe")
local proj = require("project")

proj.configure_all()
proj.stop_after("1s")
proj.set_realtime_factor(-1)

cloe.schedule {
    on = "loop",
    pin = true,
    run = function()
        ARRAY_SIZE = 1000

        local array = {}
        for i = 1, ARRAY_SIZE do
            array[i] = math.random()
        end
        table.sort(array)
    end
}
