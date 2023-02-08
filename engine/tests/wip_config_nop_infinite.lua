local cloe = require("cloe")
assert(cloe.has_feature("cloe-0.21"))

-- Nothing is available until it is loaded.
cloe.load_plugin("builtin://nop-simulator", "nop")
local plugins = cloe.util.split(os.getenv("CLOE_PLUGIN_PATH"), ":")
for _, p in ipairs(plugins) do
    cloe.load_plugin(p)
end

cloe.setup_plugin {
    plugin = "nop"
}

cloe.setup_vehicle({
    name = "default",
    from = {
        simulator = "nop", -- cloe.plugins.nop
        index = 0
    },
    components = {
        ["cloe::speedometer"] = {
            binding = "speedometer",
            name = "default_speed",
            from = "cloe::gndtruth_ego_sensor"
        },
        ["cloe::default_world_sensor"] = {
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
        },
    }
})

cloe.setup_plugin {
    plugin = "virtue",
    vehicle = "default"
}

cloe.setup_plugin {
    plugin = "basic",
    vehicle = "default"
}

cloe.schedule_test {
    id = "0d5a3cc7-4203-42d1-b50a-c1ea618cc92e",
    name = "test_basic",
    on = "start",
    desc = "use the ACC functionality from the basic controller",
    plot_signals = {
        "blabla_stupid_long_m_asdf_asdM_1_2__signal_signal_duplicate2",
        "blabla_stupid_long_m_asdf_asdM_1_2__signal_signal_duplicate3",
        "blabla_stupid_long_m_asdf_asdM_1_2__signal_signal_duplicate4",
        "blabla_stupid_long_m_asdf_asdM_1_2__signal_signal_duplicate5",
        "blabla_stupid_long_m_asdf_asdM_1_2__signal_signal_duplicate6",
    },
    run = function(z)
        local hmi = cloe.plugins.basic.hmi

        hmi.enable = false
        z.wait_until(function()
            return cloe.vehicles["default"]["speed"] > 50;
        end)
        z.wait_duration("1s")
        hmi.enable = true
        z.wait_duration("5s")

        z.comment("Press resume button")
        hmi.resume = true
        z.assert(hmi.resume == true, "Pressing the resume button works")
        z.wait_duration("0.5s")
        hmi.resume = false

        cloe.debug()
        z.wait_duration("1s")

        -- Helper function
        z.push_button = function(what, dur)
            hmi[what] = true
            z.wait_duration(dur)
            hmi[what] = false
        end

        -- Press plus button
        z.push_button("plus", "1s")
    end,
}
