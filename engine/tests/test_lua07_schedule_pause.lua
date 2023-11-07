local cloe = require("cloe")

cloe.apply_stack {
    version = "4",
    include = {
        "config_nop_infinite.json",
    },
    engine = {
        security = {
            enable_command_action = true
        },
        -- This test case will hang in failure, so enable the watchdog.
        watchdog = {
            mode = "kill",
        }
    },
    server = {
        listen = false,
        listen_port = 23456,
    },
}

cloe.schedule_these {
    on = cloe.events.start(),
    {
        -- Resume from outside
        run = {
            name = "command",
            mode = "async",
            command = [[
                curl --retry 10 --retry-connrefused --retry-delay 1 localhost:7890
            ]]
        }
    },
    {
        -- Pause
        run = {
            name = "command",
            command = [[
                echo "Resume with: curl localhost:7890";
                echo OK | netcat -l localhost 7890
            ]]
        }
    }
}

-- Alternate implementation wih Lua API
local system = require("cloe.system")
cloe.schedule_these {
    on = cloe.events.time("500ms"),
    {
        -- Resume from outside
        run = function()
            system.exec {
                command = "curl --retry 10 --retry-connrefused --retry-delay 1 localhost:7892",
                mode = "detach"
            }
        end
    },
    {
        -- Pause
        run = function()
            system.exec [[
                echo "Resume with: curl localhost:7892";
                echo OK | netcat -l localhost 7892
            ]]
        end
    }
}

cloe.schedule {
    on = cloe.events.time("1s"),
    run = cloe.actions.succeed()
}
