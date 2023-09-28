local cloe = require("cloe")
local ans, ec

ans, ec = cloe.system "echo hello world"
assert(ec == 0)
assert(ans == "hello world")

ans, ec = cloe.system {
    command = "echoxxx hello world",
    log_output = "never",
}
assert(ec ~= 0)
assert(ans == "/bin/bash: echoxxx: command not found")
