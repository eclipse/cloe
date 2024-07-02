local cloe = require("cloe")
local events, actions = cloe.events, cloe.actions

cloe.load_stackfile("config_nop_smoketest.json")

local Sig = {
    VehAcc = "vehicles.default.basic.acc"
}
cloe.require_signals_enum(Sig)
cloe.record_signals(Sig)
cloe.record_signals( {
    ["acc_config.limit_acceleration"] = function()
        return cloe.signal(Sig.VehAcc).limit_acceleration
    end,
    ["acc_config.limit_deceleration"] = function()
        return cloe.signal(Sig.VehAcc).limit_deceleration
    end,
})

-- Run a simple test.
cloe.schedule_test({
    id = "20b741ee-ef82-4638-bd61-87a3fb4221d2",
    on = events.start(),
    terminate = false,
    run = function(z, sync)
        z:wait_duration("1s")
        z:stop()
    end,
})

-- Check recording.
cloe.schedule_test {
    id = "a0065f68-2e1f-436c-8b17-fa19a630509c",
    on = events.stop(),
    run = function(z, sync)
        -- Inspect the recording in the report:
        local api = require("cloe-engine")
        local report = api.get_report()

        z:assert(report.signals ~= nil, "report.signals should not be nil")
    end
}
