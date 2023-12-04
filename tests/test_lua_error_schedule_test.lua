local cloe = require("cloe")

cloe.load_stackfile("config_nop_infinite.json")

cloe.schedule_test {
    id = "9cc0c5a4-5771-4cec-befe-ae49bd3e0cae",
    on = "start",
    run = function()
        error("expect error")
    end
}

cloe.schedule {
    desc = "This should not run.",
    on = "start",
    run = "succeed",
}
