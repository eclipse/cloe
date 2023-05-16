--
-- Copyright 2023 Robert Bosch GmbH
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--
-- SPDX-License-Identifier: Apache-2.0
--

local cloe = cloe or {}

--- @alias PluginSpec table|string

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

--- Try to load plugin, returns nil if fail, otherwise plugin object.
---
--- @param opts (PluginSpec)
--- @return nil
cloe.load_plugin = function(opts)
    return nil
end

cloe.load_file = function() end

--- Setup the scheduler
cloe.setup_scheduler = function(opts)
end

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

