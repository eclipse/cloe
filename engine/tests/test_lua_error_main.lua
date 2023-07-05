local cloe = require("cloe")

error("expect error")

cloe.schedule {
    desc = "This should not run.",
    on = "start",
    run = "succeed",
}
