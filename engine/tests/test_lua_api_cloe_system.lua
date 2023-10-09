local cloe = require("cloe")
local ans, ec

function string:endswith(suffix)
    return self:sub(-#suffix) == suffix
end

ans, ec = cloe.system "echo hello world"
assert(ec == 0)
assert(ans == "hello world")

ans, ec = cloe.system {
    command = "echoxxx hello world",
    log_output = "never",
}
assert(ec ~= 0)
assert(ans:endswith("echoxxx: command not found"))
