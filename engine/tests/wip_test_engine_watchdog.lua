-- Translation from test_engine_watchdog.json
--

local cloe = require("cloe")

cloe.require_version("0.21")
cloe.load_file("wip_config_nop_smoketest.lua")

cloe.setup_engine {
    security = {
        enabled_command_action = true
    },
    polling_interval = 60000,
    watchdog = {
        mode = "kill",
        default_timeout = 1000
    }
}

cloe.setup_server {
    enabled = false
}

cloe.schedule {
    enabled = cloe.has_feature("cloe-server"),

    { on = "time=45", run = "pause"},
    {
        desc = "Insert resume trigger via curl to test the pause-resume behavior.",
        on = "pause",
        run = {
            name = "command",
            -- This probably won't work in the future.
            command = "echo '{\"event\": \"pause\", \"action\": \"resume\"}' | curl -d @- http://localhost:23456/api/triggers/input"
      }
    }
}
