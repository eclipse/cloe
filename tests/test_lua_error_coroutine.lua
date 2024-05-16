local cloe = require("cloe")

local co = coroutine.create(function()
    error("expect error")
end)
local ok, result = coroutine.resume(co)
if not ok then
    error(result)
end

cloe.schedule {
    desc = "This should not run.",
    on = "start",
    run = "succeed",
}
