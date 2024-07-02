local sys = require("cloe.system")
local ans, ec

local function endswith(s, suffix)
    return string.sub(s, -#suffix) == suffix
end

ans, ec = sys.exec "echo hello world"
assert(ec == 0)
assert(ans == "hello world")

ans, ec = sys.exec {
    command = "echoxxx hello world",
    log_output = "never",
}
assert(ec ~= 0)
print(ans)
assert(endswith(ans, "not found"))
