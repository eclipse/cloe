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

local api = require("cloe-engine")
local validate = require("cloe.typecheck").validate

local system = {}

--- Run a command with the default system shell and return the output.
---
--- Discover the shell with:
---
---   cloe.system('echo "System shell: $0" >&2')
---
--- Note on stderr:
---   Only stdout is captured. The stderr output from the command
---   is sent straight to stderr of the calling program and not
---   discarded.
---
---   Capture stderr with:
---
---     cmd 2>&1
---
---   Discard stderr with:
---
---     cmd 2>/dev/null
---
--- @param cmd CommandSpec Command to run
--- @return string, number # Combined output, exit code
function system.exec(cmd)
  -- FIXME: This is not a very nice API...
  if type(cmd) == "string" then
    cmd = {
      command = cmd,
    }
  end
  if cmd.log_output == nil then cmd.log_output = "on_error" end
  if cmd.ignore_failure == nil then cmd.ignore_failure = true end
  return api.exec(cmd)
end

--- Return output from system call or nil on failure.
---
--- @param cmd CommandSpec
--- @return string|nil
function system.exec_or_nil(cmd)
    validate("cloe.report.exec_or_nil(CommandSpec)", cmd)
    local out, ec = system.exec(cmd)
    if ec ~= 0 then
        return nil
    end
    return out
end

--- Return system hostname.
---
--- @return string|nil
function system.get_hostname()
    -- FIXME(windows): Does `hostname` have `-f` argument in Windows?
    return system.exec_or_nil("hostname -f")
end

--- Return current username.
---
--- In a Docker container this probably doesn't provide a lot of value.
---
--- @return string|nil
function system.get_username()
    return system.exec_or_nil("whoami")
end

--- Return current date and time in RFC 3339 format.
---
--- Example: 2006-08-14 02:34:56-06:00
---
--- @return string
function system.get_datetime()
    return tostring(os.date("%Y-%m-%d %H:%M"))
end

--- Return Git hash of HEAD for the given directory path.
---
--- @param path string
--- @return string|nil
function system.get_git_hash(path)
    validate("system.get_git_hash(string)", path)
    return system.exec_or_nil({
        path = "git",
        args = {"-C", path, "rev-parse", "HEAD"}
    })
end

return system
