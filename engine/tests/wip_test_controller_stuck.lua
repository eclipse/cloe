-- Translation from test_engine_stuck_controller.json
--

local cloe = require("cloe")

cloe.load_plugin("builtin://nop-controller", "nop")
cloe.load_plugin("demo_stuck")

cloe.setup_plugin {
    plugin = "nop"
}

cloe.setup_vehicle {
    name = "default",
    from = {
        simulator = "nop",
        index = 0
    }
}

cloe.setup_plugin {
    plugin = "demo_stuck",
    vehicle = "default",
    args = {
        progress_per_step = 10000
    }
}

cloe.setup_simulation {
    controller_retry_limit = 100
}

cloe.schedule {
    { on = "start",   run = "log=info: Running nop/demo_stuck test." },
    { on = "start",   run = "realtime_factor=-1" },
    { on = "time=60", run = "succeed" }
}
