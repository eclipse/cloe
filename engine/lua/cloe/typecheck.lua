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

local m = {}

local skip_typechecks = false

function m.disable()
    skip_typechecks = true
end

function m.validate(format, ...)
    if skip_typechecks then
        return
    end
    local fn = require("typecheck").argscheck(format, function() end)
    fn(...)
end

--- Validate the shape (from tableshape) of a table or type.
---
--- @param signature string function signature for error message
--- @param shape any shape validator
--- @param value any value to validate
--- @return nil # raises an error (level 3) if invalid
function m.validate_shape(signature, shape, value)
    if skip_typechecks then
        return
    end
    local ok, msg = shape:check_value(value)
    if not ok then
        error(signature .. ": " .. msg, 3)
    end
end

return m
