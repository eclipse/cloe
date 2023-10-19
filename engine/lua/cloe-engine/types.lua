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

---
--- @meta cloe-engine.types
---
--- This file contains the type annotations of the `cloe-engine.types` module,
--- which are exported by the cloe-engine executable.
---
--- These methods should only be used by the cloe library.
---

--- @class Stack
local Stack = {}

--- Merge JSON stackfile into simulation configuration.
---
--- @param filepath string
--- @return nil
function Stack:merge_stackfile(filepath) end

--- Merge JSON string into simulation configuration.
---
--- @param json string Input JSON (use Lua multiline feature)
--- @param source_filepath string Filepath to use for error messages
--- @return nil
function Stack:merge_stackjson(json, source_filepath) end

--- Merge Lua table into simulation configuration.
---
--- This converts the table to JSON, then loads it.
---
--- @param tbl table Input JSON as Lua table (use Lua multiline feature)
--- @param source_filepath string Filepath to use for error messages
--- @return nil
function Stack:merge_stacktable(tbl, source_filepath) end


--- @class Duration
local Duration = {}

--- Return new Duration instance from duration format.
---
--- @param format string Duration such as "1s" or "1.5 ms"
--- @return Duration
function Duration.new(format) end

--- Return Duration as nanoseconds.
---
--- @return number nanoseconds
function Duration:ns() end

--- Return Duration as microseconds.
---
--- @return number microseconds
function Duration:us() end

--- Return Duration as milliseconds.
---
--- @return number milliseconds
function Duration:ms() end

--- Return Duration as seconds.
---
--- @return number seconds
function Duration:s() end


--- @class Sync
local Sync = {}

--- Return current simulation step.
---
--- @return integer
--- @nodiscard
function Sync:step() end

--- Return simulation step_width.
---
--- @return Duration
--- @nodiscard
function Sync:step_width() end

--- Return current simulation time.
---
--- @return Duration
--- @nodiscard
function Sync:time() end

--- Return estimated simulation end.
---
--- If unknown, then 0 is returned.
---
--- @return Duration
--- @nodiscard
function Sync:eta() end

--- Return current simulation realtime-factor target.
---
--- @return number
--- @nodiscard
function Sync:realtime_factor() end

--- Return whether realtime-factor target is unlimited.
---
--- If true, then the simulation runs as fast as possible and never pads
--- cycles with waiting time.
---
--- @return boolean
--- @nodiscard
function Sync:is_realtime_factor_unlimited() end

--- Return estimated achievable simulation realtime-factor target.
---
--- @return number
--- @nodiscard
function Sync:achievable_realtime_factor() end


--- @class TriggerSchema
--- @field event string|table
--- @field action string|table|function
--- @field label? string
--- @field action_source? string
--- @field sticky? boolean
--- @field priority? number
--- @field group? string


--- @class Coordinator
local Coordinator = {}

--- Insert a trigger into the coordinator event queue.
---
--- @param trigger TriggerSchema trigger schema to insert
--- @return nil
function Coordinator:insert_trigger(trigger) end

--- Execute an action known to the coordinator immediately.
---
--- @param action string|table action schema to insert
--- @return nil
function Coordinator:execute_action(action) end


return {
    Stack = Stack,
    Duration = Duration,
    Sync = Sync,
    Coordinator = Coordinator,
}
