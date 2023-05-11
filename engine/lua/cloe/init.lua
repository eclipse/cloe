-- The `cloe` table will be made availabe by cloe-engine, but if it
-- isn't, then use an empty table instead.
--
-- This prevents us from overwriting the fields that cloe-engine sets.
local cloe = _G["cloe"] or {}

--- @alias FeatureId string

--- Return if Cloe has feature as defined by string.
---
--- @param id (FeatureId)  feature identifier, such as `cloe-0.20`
--- @return boolean
cloe.has_feature = function(id)
    return cloe.api._FEATURES[id] and true or false
end

--- Throw an exception if Cloe does not have feature as defined by string.
---
--- @param id (FeatureId)  feature identifier, such as `cloe-0.20`
--- @return nil
cloe.require_feature = function(id)
    if not cloe.has_feature(id) then
        cloe.api.throw_exception("required feature " .. id .. " is not available")
    end
end

--- Log a message with a given severity.
---
--- @param level (string) severity level, one of: trace, debug, info, warn, error, critical
--- @param fmt (string) format string with trailing arguments compatible with string.format
cloe.log = function(level, fmt, ...)
    local msg = string.format(fmt, ...)
    cloe.api.log(level, "lua", msg)
end

--- Try to load stackfile, return deserialized json.
---
--- @param file (string)
--- @return nil
cloe.load_stackfile = function(file)
    assert(cloe.api.THIS_SCRIPT_DIR)
    if cloe.fs.is_relative(file) then
        file = cloe.api.THIS_SCRIPT_DIR .. "/" .. file
    end
    cloe.api.load_stackfile(file)
end

--- Return a human-readable representation of Lua objects.
---
--- This is primarily used for debugging and should not be used
--- when performance is important. It is a table, but acts as a
--- function.
---
--- For more details, see: https://github.com/kikito/inspect.lua
---
--- @return string representation of object
cloe.inspect = require("cloe.inspect")

--- Wraps cloe.inspect with a print function for faster inspection.
cloe.describe = function(...)
    print(cloe.inspect(...))
end

return cloe
