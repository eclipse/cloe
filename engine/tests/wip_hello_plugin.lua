local cloe = require("cloe")

local plugin = (function()
    local A = {}
    A.new = function(self, conf)
        self.conf = conf
        return self
    end
    A.step = function()
        print("hello")
        return "+100ms"
    end
    A.export = {
        "step"
    }

    local F = {}
    F.name = "hello"
    F.conf = {
        cycle_length = "100ms",
        last_cycle = "1000ms"
    }
    F.setup = function(conf)
        conf = cloe.util.table_deep_extend(conf, F.conf)
        return A:new(conf)
    end

    return F
end)()

local a = plugin.setup()
cloe.schedule_agent {
    unit = a,
    name = "hello"
}
