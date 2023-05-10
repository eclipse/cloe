--- @alias PluginSpec table|string

--- @alias FeatureId string

--- @class TestSpec
--- @field id string
--- @field name? string
--- @field desc? string
--- @field enabled? boolean|fun():boolean
--- @field on? string|string[]|fun():boolean
--- @field run fun(TestFixture):boolean
--- @field reset? boolean
--- @field report? function

--- @class TestFixture
--- @field id string
--- @field name? string
--- @field desc? string
--- @field assert fun(...)
--- @field print fun(...)
--- @field succeed fun(...)
--- @field fail fun(...)
--- @field abort fun(...)

--- @alias ScheduleId integer
--- @alias ScheduleGroup string

--- @class ScheduleSpec
--- @field on string
--- @field priority? integer
--- @field run function
--- @field group? ScheduleGroup
--- @field sticky? boolean
--- @field enabled? boolean|fun():boolean

-- The `cloe` table will be made availabe by cloe-engine, but if it
-- isn't, then use an empty table instead.
local cloe = _G["cloe"] or {}

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

cloe.debug = function() end

cloe.log = function() end

--- Try to load plugin, returns nil if fail, otherwise plugin object.
---
--- @param opts (PluginSpec)
--- @return nil
cloe.load_plugin = function(opts)
    return nil
end

--- Try to load stackfile, return deserialized json.
--- @param file (string)
--- @return nil
cloe.load_stackfile = function(file)
    assert(cloe.api.THIS_SCRIPT_DIR)
    if cloe.fs.is_relative(file) then
        file = cloe.api.THIS_SCRIPT_DIR .. "/" .. file
    end
    cloe.api.load_stackfile(file)
end

cloe.load_file = function() end

cloe.setup_scheduler = function() end

cloe.setup_simulation = function() end

cloe.setup_engine = function() end

--- @param spec (table)
cloe.setup_plugin = function(spec)
    local p = spec[1]               -- plugin name or path
    local as = spec["as"] or p      -- load plugin as, this needed for require
    local name = spec["name"] or as -- agent name
    local conf = spec["conf"] or {} -- agent configuration

    -- Best be as verbose as possible for clarity's sake.
    cloe.load_plugin {
        plugin = p,
        name = as,
    }
    local factory = cloe.require(as)
    local agent = factory.setup(conf)
    cloe.schedule_agent({
        agent = agent,
        name = name
    })
end

cloe.setup_vehicle = function() end

-- M.simulation = {}
-- M.plugins = {}
-- M.vehicles = {}
-- M.server = {}

cloe.scheduler = {}

--- @param match ScheduleId|ScheduleGroup|table
--- @return nil
cloe.scheduler.remove = function(match)
    print(spec)
end

--- @param spec ScheduleSpec
--- @return nil
cloe.scheduler.insert = function(spec)
    print(spec)
end

--- Schedule a unit from a plugin.
---
--- TODO: Find a good name for "unit":
---   agent
---   block
---   component
---   entity
---   model
---   module
---   object
---   plugin
---   system
---   unit
--- @param spec ScheduleAgentSpec
--- @return nil
cloe.schedule_agent = function(opts)
    return nil
end

--- @return ScheduleId|nil
cloe.schedule = function(opts)
    cloe.scheduler.insert(opts)
end

--- @param test (TestSpec)
cloe.schedule_test = function(test)
    if test.enabled then
        if type(test.enabled) == "function" then
            if not test.enabled() then
                return false
            end
        end
    end
    cloe.schedule({
        enabled = false
    })
end

cloe.config = {
    simulation = {
        realtime_factor = 1
    },
    scheduler = {
        tick = "1ms",
    }
}

return cloe
