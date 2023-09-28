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

cloe.state.report = cloe.state.report or {}
cloe.state.report.tests = cloe.state.report.tests or {}

local m = {}

--- Return system hostname.
---
--- @return string
function m.get_hostname()
    -- FIXME(windows): Does `hostname` have `-f` argument in Windows?
    return cloe.system("hostname -f")
end

--- Return current username.
---
--- In a Docker container this probably doesn't provide a lot of value.
---
--- @return string
function m.get_username()
    return cloe.system("whoami")
end

--- Return current date and time in RFC 3339 format.
---
--- Example: 2006-08-14 02:34:56-06:00
---
--- @return string
function m.get_datetime()
    -- FIXME(windows): Does this command exist on Windows?
    return cloe.system("date --rfc-3339=seconds")
end

--- Initialize report metadata.
---
--- @param header table Optional report header information that will be merged in.
--- @return nil
function cloe.init_report(header)
    cloe.validate({
        header = { header, "table", true }
    })
    cloe.state.report.metadata = {
        hostname = m.get_hostname(),
        username = m.get_username(),
        datetime = m.get_datetime(),
    }
    if header then
        cloe.state.report.metadata = cloe.tbl_deep_extend("force", cloe.state.report.metadata, header)
    end
end

return m
