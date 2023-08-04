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
--- @meta
---
--- This file contains the type annotations of the `cloe.fs` table,
--- defined in `src/lua_setup_fs.cpp` as of this writing.
---

local cloe = cloe or {}
if cloe.fs then
    return cloe.fs
end

local fs = {}

--- Return the basename of the filepath.
---
--- Examples:
---
---     assert fs.basename("/bin/bash") == "bash"
---     assert fs.basename("c:\\path") == "c:\\path"    -- on linux
---
--- @param path string filepath
--- @return string # basename of file without parent
function fs.basename(path) end

--- Return the parent of the filepath.
---
--- Examples:
---
---     assert fs.dirname("/bin/bash") == "/bin"
---     assert fs.dirname("/") == "/"
---     assert fs.dirname("") == ""
---     assert fs.dirname("c:\\path") == ""             -- on linux
---
--- @param path string filepath
--- @return string # parent of file without basename
function fs.dirname(path) end

--- Return the normalized filepath.
---
--- Examples:
---
---     assert fs.normalize("/bin/../bin/bash") == "/bin/bash"
---     assert fs.normalize("/no/exist//.//../exists") == "/no/exists"
---
--- @param path string filepath
--- @return string # normalized file
function fs.normalize(path) end

--- Return the true filepath, resolving symlinks and normalizing.
--- If the file does not exist, an empty string is returned.
---
--- Examples:
---
---     assert fs.realpath("/bin/../bin/bash") == "/usr/bin/bash"
---     assert fs.realpath("/no/exist") == ""
---
--- @param path string filepath
--- @return string # real path of file
function fs.realpath(path) end

--- Return the left and right arguments joined together.
---
--- @param left string filepath
--- @param right string filepath
--- @return string # filepaths joined as "left/right"
function fs.join(left, right) end

--- Return whether path is an absolute path.
---
--- @param path string filepath to check
--- @return boolean # true if path is absolute
function fs.is_absolute(path) end

--- Return whether path is a relative path.
---
--- @param path string filepath to check
--- @return boolean # true if path is relative
function fs.is_relative(path) end

--- Return whether path refers to an existing directory.
---
--- Symlinks are resolved, hence is_dir(path) and is_symlink(path)
--- can both be true.
---
--- @param file string filepath to check
--- @return boolean # true if path exists and is a directory
function fs.is_dir(file) end

--- Return whether path refers to an existing normal file.
---
--- A normal file excludes block devices, pipes, sockets, etc.
--- For these files, use is_other() or exists().
--- Symlinks are resolved, hence is_file(path) and is_symlink(path)
--- can both be true.
---
--- @param file string filepath to check
--- @return boolean # true if path exists and is a normal file
function fs.is_file(file) end

--- Return whether path refers to an existing symlink.
---
--- @param file string filepath to check
--- @return boolean # true if path exists and is a symlink
function fs.is_symlink(file) end

--- Return whether path refers to something that exists,
--- but is not a file, directory, or symlink.
---
--- This can be the case if it is a block device, pipe, socket, etc.
---
--- @param file string filepath to check
--- @return boolean # true if path exists and is not a normal file, symlink, or directory
function fs.is_other(file) end

--- Return whether path refers to something that exists,
--- regardless what it is.
---
--- @param file string filepath to check
--- @return boolean # true if path exists
function fs.exists(file) end

return fs
