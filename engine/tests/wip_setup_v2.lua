-- High-level plugin loading and setup.
--
-- We are assuming that cloe has the plugin search path set via environment
-- or that the plugins reside in their normal locations.
local cloe = require("cloe")

cloe.setup_plugin {
    "minimator",
    conf = {
        vehicles = {
            "default"
        }
    }
}

cloe.setup_plugin {
    "speedometer",
    name = "default_speed"
}

cloe.setup_plugin {
    "speedometer",
    name = "dummy_speed"
}

cloe.setup_plugin {
    "basic"
}
