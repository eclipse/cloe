local sys = require("cloe.system")
local ans, ec

function string:endswith(suffix)
    return self:sub(-#suffix) == suffix
end

ans, ec = sys.exec "echo hello world"
assert(ec == 0)
assert(ans == "hello world")

ans, ec = sys.exec {
    command = "echoxxx hello world",
    log_output = "never",
}
assert(ec ~= 0)
assert(ans:endswith("echoxxx: command not found"))
