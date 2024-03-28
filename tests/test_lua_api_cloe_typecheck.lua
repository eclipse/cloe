local validate = require("cloe.typecheck").validate
local argscheck = require("typecheck").argscheck

local function log(fmt, ...)
    validate("log(string, [?any]...)", fmt, ...)
    print(string.format(fmt, ...))
end

log("hello world")
log("hello %s", "you")
log("hello %s and %s", "you", "me")
