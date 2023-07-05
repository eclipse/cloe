local cloe = require("cloe")

-- which will schedule itself
cloe.load_plugin("vtd")
cloe.load_plugin("virtue")
require("vtd").setup(cloe.config.simulator.vtd)

assert(not cloe.has_feature("cloe-server"))
if cloe.has_feature("cloe-server") then
    cloe.server.setup({
        enabled = true,
        listen_address = "0.0.0.0",
        listen_port = 23456,
    })
end

cloe.setup_scheduler {
    step_size = "1ms"
}

cloe.schedule {
    on = "step",
    priority = 1000,
    run = function (sync)
        cloe.plugins.vtd.process(sync)
    end,
}

cloe.schedule {
    on = "post_step",
    run = function (sync)
        cloe.plugins.realtime.process(sync)
    end,
}

cloe.schedule {
    on = "start",
    run = function()
        cloe.simulation.realtime_factor = -1
    end
}

cloe.schedule {
    { on = "start", run = "log=info: Running nop/basic smoketest" },
}

-- Multiple actions can be scheduled 
cloe.schedule {
    group = "fail_conditions",
    desc = "determine whether simulation is fail or success",
    {
        on = "step",
        run = function (_)
            if cloe.vehicle.default["speed_kmph"] > 100 then
                cloe.debug()
                cloe.simulation.fail("vehicle speed exceeded safe limit")
            end
        end
    },
    {
        on = "virtue.failure",
        run = {
            "log=error: Failed virtue test",
            "fail",
        }
    },
}

cloe.schedule { on = "time=60s", run = "succeed" }

cloe.schedule_test {
    -- @class TestSpec
    -- All events scheduled from within this test are given the UUID as the group,
    -- which makes it easy to delete them when the test is over.

    -- @field id string|fun():string
    id = "b653c3b0-5d29-4b91-b493-d72578ca62ad",

    -- @field name? string|fun():string = ""
    name = "asdf",

    -- @field desc? string = ""
    desc = "Make sure we can enable and disable Basic ACC",

    -- @field enabled? boolean|fun():boolean = true
    enable = true,

    -- @type string|string[]|fun():boolean
    on = "start",

    -- Run this as a coroutine, which lets us yield until...
    -- @field fun(TestFixture)
    run = function(test)
        local basic = cloe.vehicle.default["basic"]

        basic.set_speed(80.0)

        -- This schedules a resumption and then uses coroutine.yield to let cloe continue
        --
        --     cloe.schedule({
        --         on = function() return cloe.vehicle.default["speed_kmph"] > 50 end,
        --         run = function() coroutine.resume(test.coroutine_id) end,
        --         group = test.id,
        --     })
        --     coroutine.yield()
        --
        cloe.wait_until(function() return cloe.vehicle.default["speed_kmph"] > 50.0 end)
        local time = cloe.state.time

        -- It should not take longer than 10 simulated seconds to get from 50 to 80
        local index = cloe.wait_until({
            -- Either succeed:
            function() return cloe.vehicle.default["speed_kmph"] >= 79.0 end,

            -- Or timeout:
            "next=10s",

            needs = "any" -- or "all"
        })

        if index == 1 then
            test.assert(cloe.vehicle.default["speed_kmph"] >= 79.0)
            test.succeed()
        else
            test.assert(cloe.state.time >= time + 10)
            test.fail()
        end
    end,

    report = function() end,
}
