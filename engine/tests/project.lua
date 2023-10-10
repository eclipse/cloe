-- This file configures the "project", as it were.
--
-- It more or less does the exact same as config_nop_smoketest.json,
-- except in a more configurable, modular way.
--
-- You will notice that this file follows the same format as a Lua
-- module, and indeed, this is how it is expected to be used:
--
--      local project = require("project")
--      project.configure_all {
--          with_server = false,
--          with_noisy_sensor = true,
--      }
--
-- In order to be maximally useful to users, each of the defined
-- functions should be documented so that the Lua Language Server
-- can give the users auto-completion and documentation hints.
local cloe = require("cloe")

local m = {}

--- Apply a stackfile, setting version to "4".
---
--- These lines modify the input stack to make it
--- conform to the current stackfile version.
--- Call it a minimal "quality-of-life improvement".
---
--- @param spec table Stack input table
--- @return nil -- Return value of cloe.apply_stack
m.apply_stack = function(spec)
    cloe.validate({
        spec = {spec, "table"}
    })
    spec.version = "4"
    return cloe.apply_stack(spec)
end

--- @class ProjectOptions 
--- @field with_server? boolean
--- @field with_noisy_sensor boolean

--- Configure all aspects of the simulation.
---
--- This just calls the other configure_* methods in a convenient way.
---
--- @param opts ProjectOptions
--- @return nil
m.configure_all = function(opts)
    cloe.validate({opts = {opts, "table", true }})
    opts = opts or {}
    cloe.validate({
        with_server = { opts.with_server, "boolean", true },
        with_noisy_sensor = { opts.with_noisy_sensor, "boolean", true },
    })

    local vehname = "default"
    local simname = "nop"

    m.configure_nop_simulator(simname)
    m.configure_vehicle(vehname, simname, {
        with_noisy_sensor = opts.with_noisy_sensor
    })
    m.configure_server(opts.with_server)
    m.configure_virtue(vehname)
    m.configure_basic(vehname)
end

--- Configure the simulator.
---
--- @param name string Name of simulator (e.g. "sim")
--- @return nil -- Return value of cloe.apply_stack
m.configure_nop_simulator = function(name)
    cloe.validate({
        name = {name, "string"}
    })

    return m.apply_stack {
        simulators = {
            { binding = "nop", name = name }
        }
    }
end

--- @class VehicleOptions
--- @field with_noisy_sensor? boolean

--- Configure the vehicle.
---
--- @param name string Name of the vehicle
--- @param simulator string|table Name of simulator or config block
--- @param opts VehicleOptions
--- @return nil -- Return value of cloe.apply_stack
m.configure_vehicle = function(name, simulator, opts)
    cloe.validate({
        name = {name, "string"},
        simulator = {simulator, {"string", "table"}},
        opts = {opts, "table", true},
    })
    local from = simulator
    if type(from) == "string" then
        from = {
            simulator = simulator,
            index = 0
        }
    end
    opts = opts or {}
    cloe.validate({
        with_noisy_sensor = { opts.with_noisy_sensor, "boolean", true }
    })

    local components = {
        ["cloe::speedometer"] = {
            binding = "speedometer",
            name = "default_speed",
            from = "cloe::gndtruth_ego_sensor"
        }
    }
    if opts.with_noisy_sensor then
        components["cloe::default_world_sensor"] = {
            binding = "noisy_object_sensor",
            name = "noisy_object_sensor",
            from = "cloe::default_world_sensor",
            args = {
                noise = {
                    {
                        target = "translation",
                        distribution = {
                            binding = "normal",
                            mean = 0.0,
                            std_deviation = 0.3
                        }
                    }
                }
            }
        }
    end

    if from.simulator == "nop" then
        cloe.schedule {
            desc = "Vehicle should never move with nop binding",
            on = "default_speed/kmph=>0.0",
            run = "fail"
        }
    end

    return m.apply_stack {
        vehicles = {
            { name = name, from = from, components = components }
        }
    }
end

--- Configure server if possible.
---
--- @param enable boolean
--- @return nil
m.configure_server = function(enable)
    if enable then
        -- Query system to see if something is already listening on the port.
        local code = os.execute("ss -H -l 'sport = 23456' | grep tcp")
        if code == 0 then
            cloe.log("error", "a process is already listening at 23456")
            enable = false
        end
    end
    return m.apply_stack {
        server = {
            listen = enable,
            listen_port = 23456
        }
    }
end

--- Configure virtue controller for vehicle.
---
--- @param vehicle string Vehicle name
--- @return nil
m.configure_virtue = function(vehicle)
    m.apply_stack {
        controllers = {
            { binding = "virtue", vehicle = vehicle }
        }
    }
    cloe.schedule { on = "virtue/failure", run = "fail" }
end

--- Configure basic controller for vehicle.
---
--- @param vehicle string Vehicle name
--- @return nil
m.configure_basic = function(vehicle)
    m.apply_stack {
        controllers = {
            { binding = "basic", vehicle = vehicle }
        }
    }
    cloe.schedule_these {
        { on = "start", run = "basic/hmi=!enable" },
        { on = "next=1", run = "basic/hmi=enable" },
        { on = "time=5", run = "basic/hmi=resume" },
        { on = "time=5.5", run = "basic/hmi=!resume" },
    }
end

--- Set realtime factor.
---
--- @param factor number Use -1 for maximum speed, 1.0 for realtime
--- @return nil
m.set_realtime_factor = function(factor)
    if factor == 0 then
        error "cannot set realtime factor of 0"
    end
    cloe.schedule { on = "start", run = "realtime_factor="..tostring(factor) }
end

--- Do an action after given duration.
---
--- @param duration string Duration with unit of time, e.g. "5s" or "5000ms"
--- @param action string|function Anything that can be scheduled
--- @return nil
m.action_after = function(duration, action)
    local dur = cloe.Duration.new(duration)
    return cloe.schedule { on = "next=" .. dur:s(), run = action }
end

--- Fail after this amount of time.
m.fail_after = function(duration) return m.action_after(duration, "fail") end

--- Succeed after this amount of time.
m.succeed_after = function(duration) return m.action_after(duration, "succeed") end

--- Stop after this amount of time.
m.stop_after = function(duration) return m.action_after(duration, "stop") end

return m
