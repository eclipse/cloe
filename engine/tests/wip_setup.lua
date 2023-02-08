-- Low-level plugin loading and setup.
--
-- We are assuming that cloe has the plugin search path set via environment
-- or that the plugins reside in their normal locations.
local cloe = require("cloe")

do
    cloe.load_plugin("minimator")
    local a = require("minimator").setup {
        vehicles = {
            "default"
        }
    }
    cloe.schedule_agent {
        agent = a,
        name = "minimator",
    }

    print("Module " .. a .. " input:")
    for key in pairs(a.input) do
        print("", key)
    end
    print("Module " .. a .. " outputs:")
    for key in pairs(a.output) do
        print("", key)
    end
end

do
    cloe.load_plugin("speedometer")
    local a = require("speedometer").setup()
    cloe.schedule_agent {
        agent = a,
        name = "default_speed"
    }
    cloe.pipe(
        cloe.agents.minimator.default.ego_sensor,
        cloe.agents.default_speed.ego_sensor
    )
end

do
    local m = require("speedometer").setup()
    cloe.schedule_agent {
        agent = m,
        name = "dummy_speed",
    }
    cloe.pipe(
        {
            pipe = "output",
            async = false,
            name = "minimator.vehicles.default.ego_sensor",
            type = "user description of function",
            func = cloe.agents.minimator.vehicles.default.ego_sensor.func,
        },
        cloe.agents.dummy_speed.ego_sensor
    )
end

do
    cloe.load_plugin("basic")
    local m = require("basic").setup()
    cloe.schedule_agent {
        agent = m,
        name = "basic",
    }

    print("Module " .. m .. " input:")
    for key in pairs(m.input) do
        print("", key)
    end
    print("Module " .. m .. " outputs:")
    for key in pairs(m.output) do
        print("", key)
    end

    cloe.pipe("minimator.default.ego_sensor", "basic.ego_sensor")
end
