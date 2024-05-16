--[[
 Source: https://github.com/gvvaughan/typecheck/blob/v3.0/lib/typecheck/init.lua
 Modifications:
 - Rename typecheck/init.lua -> typecheck.lua.
 - Embed LICENSE.md into this file.
 - Add this comment.
]]

--[[
 Gradual Function Type Checking for Lua 5.1, 5.2, 5.3 & 5.4
 Copyright (C) 2014-2023 Gary V. Vaughan

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in  all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGE-
 MENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
]]

--[[--
 Gradual type checking for Lua functions.

 The behaviour of the functions in this module are controlled by the value
 of the `argcheck` field maintained by the `std._debug` module.  Not setting
 a value prior to loading this module is equivalent to having `argcheck = true`.

 The first line of Lua code in production quality applications that value
 execution speed over rigorous function type checking should be:

    require 'std._debug' (false)

 Alternatively, if your project also depends on other `std._debug` hints
 remaining enabled:

    require 'std._debug'.argcheck = false

 This mitigates almost all of the overhead of type checking with the
 functions from this module.

 @module typecheck
]]



--[[ ====================== ]]--
--[[ Load optional modules. ]]--
--[[ ====================== ]]--


local _debug = (function()
   local ok, r = pcall(require, 'std._debug')
   if not ok then
      r = setmetatable({
         -- If this module was required, but there's no std._debug, safe to
         -- assume we do want runtime argchecks!
         argcheck = true,
         -- Similarly, if std.strict is available, but there's no _std.debug,
         -- then apply strict global symbol checks to this module!
         strict = true,
      }, {
         __call = function(self, x)
            self.argscheck = (x ~= false)
         end,
      })
   end

   return r
end)()


local strict = (function()
   local setfenv = rawget(_G, 'setfenv') or function() end

   -- No strict global symbol checks with no std.strict module, even
   -- if we found std._debug and requested that!
   local r = function(env, level)
      setfenv(1+(level or 1), env)
      return env
   end

   if _debug.strict then
      -- Specify `.init` submodule to make sure we only accept
      -- lua-stdlib/strict, and not the old strict module from
      -- lua-stdlib/lua-stdlib.
      local ok, m = pcall(require, 'std.strict.init')
      if ok then
         r = m
      end
   end
   return r
end)()


local _ENV = strict(_G)



--[[ ================== ]]--
--[[ Lua normalization. ]]--
--[[ ================== ]]--


local concat = table.concat
local find = string.find
local floor = math.floor
local format = string.format
local gsub = string.gsub
local insert = table.insert
local io_type = io.type
local match = string.match
local remove = table.remove
local sort = table.sort
local sub = string.sub


-- Return callable objects.
-- @function callable
-- @param x an object or primitive
-- @return *x* if *x* can be called, otherwise `nil`
-- @usage
--   (callable(functable) or function()end)(args, ...)
local function callable(x)
   -- Careful here!
   -- Most versions of Lua don't recurse functables, so make sure you
   -- always put a real function in __call metamethods.  Consequently,
   -- no reason to recurse here.
   -- func=function() print 'called' end
   -- func() --> 'called'
   -- functable=setmetatable({}, {__call=func})
   -- functable() --> 'called'
   -- nested=setmetatable({}, {__call=function(self, ...) return functable(...)end})
   -- nested() -> 'called'
   -- notnested=setmetatable({}, {__call=functable})
   -- notnested()
   -- --> stdin:1: attempt to call global 'nested' (a table value)
   -- --> stack traceback:
   -- -->	stdin:1: in main chunk
   -- -->		[C]: in ?
   if type(x) == 'function' or (getmetatable(x) or {}).__call then
      return x
   end
end


-- Return named metamethod, if callable, otherwise `nil`.
-- @param x item to act on
-- @string n name of metamethod to look up
-- @treturn function|nil metamethod function, if callable, otherwise `nil`
local function getmetamethod(x, n)
   return callable((getmetatable(x) or {})[n])
end


-- Length of a string or table object without using any metamethod.
-- @function rawlen
-- @tparam string|table x object to act on
-- @treturn int raw length of *x*
-- @usage
--    --> 0
--    rawlen(setmetatable({}, {__len=function() return 42}))
local function rawlen(x)
   -- Lua 5.1 does not implement rawlen, and while # operator ignores
   -- __len metamethod, `nil` in sequence is handled inconsistently.
   if type(x) ~= 'table' then
      return #x
   end

   local n = #x
   for i = 1, n do
      if x[i] == nil then
         return i -1
      end
   end
   return n
end


-- Deterministic, functional version of core Lua `#` operator.
--
-- Respects `__len` metamethod (like Lua 5.2+).   Otherwise, always return
-- one less than the lowest integer index with a `nil` value in *x*, where
-- the `#` operator implementation might return the size of the array part
-- of a table.
-- @function len
-- @param x item to act on
-- @treturn int the length of *x*
-- @usage
--    x = {1, 2, 3, nil, 5}
--    --> 5 3
--    print(#x, len(x))
local function len(x)
   return (getmetamethod(x, '__len') or rawlen)(x)
end


-- Return a list of given arguments, with field `n` set to the length.
--
-- The returned table also has a `__len` metamethod that returns `n`, so
-- `ipairs` and `unpack` behave sanely when there are `nil` valued elements.
-- @function pack
-- @param ... tuple to act on
-- @treturn table packed list of *...* values, with field `n` set to
--    number of tuple elements (including any explicit `nil` elements)
-- @see unpack
-- @usage
--    --> 5
--    len(pack(nil, 2, 5, nil, nil))
local pack = (function(f)
   local pack_mt = {
      __len = function(self)
         return self.n
      end,
   }

   local pack_fn = f or function(...)
      return {n=select('#', ...), ...}
   end

   return function(...)
      return setmetatable(pack_fn(...), pack_mt)
   end
end)(rawget(_G, "pack"))


-- Like Lua `pairs` iterator, but respect `__pairs` even in Lua 5.1.
-- @function pairs
-- @tparam table t table to act on
-- @treturn function iterator function
-- @treturn table *t*, the table being iterated over
-- @return the previous iteration key
-- @usage
--    for k, v in pairs {'a', b='c', foo=42} do process(k, v) end
local pairs = (function(f)
   if not f(setmetatable({},{__pairs=function() return false end})) then
      return f
   end

   return function(t)
      return(getmetamethod(t, '__pairs') or f)(t)
   end
end)(pairs)


-- Convert a number to an integer and return if possible, otherwise `nil`.
-- @function math.tointeger
-- @param x object to act on
-- @treturn[1] integer *x* converted to an integer if possible
-- @return[2] `nil` otherwise
local tointeger = (function(f)
   if f == nil then
      -- No host tointeger implementationm use our own.
      return function(x)
         if type(x) == 'number' and x - floor(x) == 0.0 then
            return x
         end
      end

   elseif f '1' ~= nil then
      -- Don't perform implicit string-to-number conversion!
      return function(x)
         if type(x) == 'number' then
            return f(x)
         end
      end
   end

   -- Host tointeger is good!
   return f
end)(math.tointeger)


-- Return 'integer', 'float' or `nil` according to argument type.
--
-- To ensure the same behaviour on all host Lua implementations,
-- this function returns 'float' for integer-equivalent floating
-- values, even on Lua 5.3.
-- @function math.type
-- @param x object to act on
-- @treturn[1] string 'integer', if *x* is a whole number
-- @treturn[2] string 'float', for other numbers
-- @return[3] `nil` otherwise
local math_type = math.type or function(x)
   if type(x) == 'number' then
      return tointeger(x) and 'integer' or 'float'
   end
end


-- Get a function or functable environment.
--
-- This version of getfenv works on all supported Lua versions, and
-- knows how to unwrap functables.
-- @function getfenv
-- @tparam function|int fn stack level, C or Lua function or functable
--    to act on
-- @treturn table the execution environment of *fn*
-- @usage
--    callers_environment = getfenv(1)
local getfenv = (function(f)
   local debug_getfenv = debug.getfenv
   local debug_getinfo = debug.getinfo
   local debug_getupvalue = debug.getupvalue

   if debug_getfenv then

      return function(fn)
         local n = tointeger(fn or 1)
         if n then
            if n > 0 then
               -- Adjust for this function's stack frame, if fn is non-zero.
               n = n + 1
            end

            -- Return an additional nil result to defeat tail call elimination
            -- which would remove a stack frame and break numeric *fn* count.
            return f(n), nil
         end

         if type(fn) ~= 'function' then
            -- Unwrap functables:
            -- No need to recurse because Lua doesn't support nested functables.
            -- __call can only (sensibly) be a function, so no need to adjust
            -- stack frame offset either.
            fn =(getmetatable(fn) or {}).__call or fn
         end

         -- In Lua 5.1, only debug.getfenv works on C functions; but it
         -- does not work on stack counts.
         return debug_getfenv(fn)
      end

   else

      -- Thanks to http://lua-users.org/lists/lua-l/2010-06/msg00313.html
      return function(fn)
         if fn == 0 then
            return _G
         end
         local n = tointeger(fn or 1)
         if n then
            fn = debug_getinfo(n + 1, 'f').func
         elseif type(fn) ~= 'function' then
            fn = (getmetatable(fn) or {}).__call or fn
         end

         local name, env
         local up = 0
         repeat
            up = up + 1
            name, env = debug_getupvalue(fn, up)
         until name == '_ENV' or name == nil
         return env
      end

   end
end)(rawget(_G, 'getfenv'))


-- Set a function or functable environment.
--
-- This version of setfenv works on all supported Lua versions, and
-- knows how to unwrap functables.
-- @function setfenv
-- @tparam function|int fn stack level, C or Lua function or functable
--    to act on
-- @tparam table env new execution environment for *fn*
-- @treturn function function acted upon
-- @usage
--    function clearenv(fn) return setfenv(fn, {}) end
local setfenv = (function(f)
   local debug_getinfo = debug.getinfo
   local debug_getupvalue = debug.getupvalue
   local debug_setfenv = debug.setfenv
   local debug_setupvalue = debug.setupvalue
   local debug_upvaluejoin = debug.upvaluejoin

   if debug_setfenv then

      return function(fn, env)
         local n = tointeger(fn or 1)
         if n then
            if n > 0 then
               n = n + 1
            end
            return f(n, env), nil
         end
         if type(fn) ~= 'function' then
            fn =(getmetatable(fn) or {}).__call or fn
         end
         return debug_setfenv(fn, env)
      end

   else

      -- Thanks to http://lua-users.org/lists/lua-l/2010-06/msg00313.html
      return function(fn, env)
         local n = tointeger(fn or 1)
         if n then
            if n > 0 then
               n = n + 1
            end
            fn = debug_getinfo(n, 'f').func
         elseif type(fn) ~= 'function' then
            fn =(getmetatable(fn) or {}).__call or fn
         end

         local up, name = 0, nil
         repeat
            up = up + 1
            name = debug_getupvalue(fn, up)
         until name == '_ENV' or name == nil
         if name then
            debug_upvaluejoin(fn, up, function() return name end, 1)
            debug_setupvalue(fn, up, env)
         end
         return n ~= 0 and fn or nil
      end

   end
end)(rawget(_G, 'setfenv'))


-- Either `table.unpack` in newer-, or `unpack` in older Lua implementations.
-- Always defaulting to full packed table unpacking when no index arguments
-- are passed.
-- @function unpack
-- @tparam table t table to act on
-- @int[opt=1] i first index to unpack
-- @int[opt=len(t)] j last index to unpack
-- @return ... values of numeric indices of *t*
-- @see pack
-- @usage
--    local a, b, c = unpack(pack(nil, 2, nil))
--    assert(a == nil and b == 2 and c == nil)
local unpack = (function(f)
   return function(t, i, j)
      return f(t, tointeger(i) or 1, tointeger(j) or len(t))
   end
end)(rawget(_G, "unpack") or table.unpack)



--[[ ================= ]]--
--[[ Helper Functions. ]]--
--[[ ================= ]]--


local function copy(dest, src)
   if src == nil then
      dest, src = {}, dest
   end
   for k, v in pairs(src) do
      dest[k] = v
   end
   return dest
end


local function split(s, sep)
   local r, pattern = {}, nil
   if sep == '' then
      pattern = '(.)'
      r[#r + 1] = ''
   else
      pattern = '(.-)' ..(sep or '%s+')
   end
   local b, slen = 0, len(s)
   while b <= slen do
      local _, n, m = find(s, pattern, b + 1)
      r[#r + 1] = m or sub(s, b + 1, slen)
      b = n   or slen + 1
   end
   return r
end



--[[ ================== ]]--
--[[ Argument Checking. ]]--
--[[ ================== ]]--


-- There's an additional stack frame to count over from inside functions
-- with argchecks enabled.
local ARGCHECK_FRAME = 0


local function argerror(name, i, extramsg, level)
   level = tointeger(level) or 1
   local s = format("bad argument #%d to '%s'", tointeger(i), name)
   if extramsg ~= nil then
      s = s .. ' (' .. extramsg .. ')'
   end
   error(s, level > 0 and level + 2 + ARGCHECK_FRAME or 0)
end


-- A rudimentary argument type validation decorator.
--
-- Return the checked function directly if `_debug.argcheck` is reset,
-- otherwise use check function arguments using predicate functions in
-- the corresponding position in the decorator call.
-- @function checktypes
-- @string name function name to use in error messages
-- @tparam funct predicate return true if checked function argument is
--    valid, otherwise return nil and an error message suitable for
--    *extramsg* argument of @{argerror}
-- @tparam func ... additional predicates for subsequent checked
--    function arguments
-- @raises argerror when an argument validator returns failure
-- @see argerror
-- @return function
-- @usage
--    local unpack = checktypes('unpack', types.table) ..
--    function(t, i, j)
--       return table.unpack(t, i or 1, j or #t)
--    end
local checktypes = (function()
   -- Set checktypes according to whether argcheck was required by _debug.
   if _debug.argcheck then

      ARGCHECK_FRAME = 1

      local function icalls(checks, argu)
         return function(state, i)
            if i < state.checks.n then
               i = i + 1
               local r = pack(state.checks[i](state.argu, i))
               if r.n > 0 then
                  return i, r[1], r[2]
               end
               return i
            end
         end, {argu=argu, checks=checks}, 0
      end

      return function(name, ...)
         return setmetatable(pack(...), {
            __concat = function(checks, inner)
               if not callable(inner) then
                  error("attempt to annotate non-callable value with 'checktypes'", 2)
               end
               return function(...)
                  local argu = pack(...)
                  for i, expected, got in icalls(checks, argu) do
                     if got or expected then
                        local buf, extramsg = {}, nil
                        if expected then
                           got = got or ('got ' .. type(argu[i]))
                           buf[#buf +1] = expected .. ' expected, ' .. got
                        elseif got then
                           buf[#buf +1] = got
                        end
                        if #buf > 0 then
                           extramsg = concat(buf)
                        end
                        return argerror(name, i, extramsg, 3), nil
                     end
                  end
                  -- Tail call pessimisation: inner might be counting frames,
                  -- and have several return values that need preserving.
                  -- Different Lua implementations tail call under differing
                  -- conditions, so we need this hair to make sure we always
                  -- get the same number of stack frames interposed.
                  local results = pack(inner(...))
                  return unpack(results, 1, results.n)
               end
            end,
         })
      end

   else

      -- Return `inner` untouched, for no runtime overhead!
      return function(...)
         return setmetatable({}, {
            __concat = function(_, inner)
               return inner
            end,
         })
      end

   end
end)()


local function resulterror(name, i, extramsg, level)
   level = level or 1
   local s = format("bad result #%d from '%s'", i, name)
   if extramsg ~= nil then
      s = s .. ' (' .. extramsg .. ')'
   end
   error(s, level > 0 and level + 1 + ARGCHECK_FRAME or 0)
end



--[[ ================= ]]--
--[[ Type annotations. ]]--
--[[ ================= ]]--


local function fail(expected, argu, i, got)
   if i > argu.n then
      return expected, 'got no value'
   elseif got ~= nil then
      return expected, 'got ' .. got
   end
   return expected
end


--- Low-level type conformance check helper.
--
-- Use this, with a simple @{Predicate} function, to write concise argument
-- type check functions.
-- @function check
-- @string expected name of the expected type
-- @tparam table argu a packed table (including `n` field) of all arguments
-- @int i index into *argu* for argument to action
-- @tparam Predicate predicate check whether `argu[i]` matches `expected`
-- @usage
--    function callable(argu, i)
--       return check('string', argu, i, function(x)
--          return type(x) == 'string'
--       end)
--    end
local function check(expected, argu, i, predicate)
   local arg = argu[i]
   local ok, got = predicate(arg)
   if not ok then
      return fail(expected, argu, i, got)
   end
end


local function _type(x)
   return (getmetatable(x) or {})._type or io_type(x) or math_type(x) or type(x)
end


local types = setmetatable({
   -- Accept argu[i].
   accept = function() end,

   -- Reject missing argument *i*.
   arg = function(argu, i)
      if i > argu.n then
         return 'no value'
      end
   end,

   -- Accept function valued or `__call` metamethod carrying argu[i].
   callable = function(argu, i)
      return check('callable', argu, i, callable)
   end,

   -- Accept argu[i] if it is an integer valued number
   integer = function(argu, i)
      local value = argu[i]
      if type(tonumber(value)) ~= 'number' then
         return fail('integer', argu, i)
      end
      if tointeger(value) == nil then
         return nil, _type(value) .. ' has no integer representation'
      end
   end,

   -- Accept missing argument *i* (but not explicit `nil`).
   missing = function(argu, i)
      if i <= argu.n then
         return nil
      end
   end,

   -- Accept non-nil valued argu[i].
   value = function(argu, i)
      if i > argu.n then
         return 'value', 'got no value'
      elseif argu[i] == nil then
         return 'value'
      end
   end,
}, {
   __index = function(_, k)
      -- Accept named primitive valued argu[i].
      return function(argu, i)
         return check(k, argu, i, function(x)
            return type(x) == k
         end)
      end
   end,
})


local function any(...)
   local fns = {...}
   return function(argu, i)
      local buf = {}
      local expected, got, r
      for _, predicate in ipairs(fns) do
         r = pack(predicate(argu, i))
         expected, got = r[1], r[2]
         if r.n == 0 then
            -- A match!
            return
         elseif r.n == 2 and expected == nil and #got > 0 then
            -- Return non-type based mismatch immediately.
            return expected, got
         elseif expected ~= 'nil' then
            -- Record one of the types we would have matched.
            buf[#buf + 1] = expected
         end
      end
      if #buf == 0 then
         return got
      elseif #buf > 1 then
         sort(buf)
         buf[#buf -1], buf[#buf] = buf[#buf -1] .. ' or ' .. buf[#buf], nil
      end
      expected = concat(buf, ', ')
      if got ~= nil then
         return expected, got
      end
      return expected
   end
end


local function opt(...)
   return any(types['nil'], ...)
end



--[[ =============================== ]]--
--[[ Implementation of value checks. ]]--
--[[ =============================== ]]--


local function xform_gsub(pattern, replace)
   return function(s)
      return (gsub(s, pattern, replace))
   end
end


local ORCONCAT_XFORMS = {
   xform_gsub('#table', 'non-empty table'),
   xform_gsub('#list', 'non-empty list'),
   xform_gsub('functor', 'functable'),
   xform_gsub('list of', '\t%0'), -- tab sorts before any other printable
   xform_gsub('table of', '\t%0'),
}


--- Concatenate a table of strings using ', ' and ' or ' delimiters.
-- @tparam table alternatives a table of strings
-- @treturn string string of elements from alternatives delimited by ', '
--    and ' or '
local function orconcat(alternatives)
   if len(alternatives) > 1 then
      local t = copy(alternatives)
      sort(t, function(a, b)
         for _, fn in ipairs(ORCONCAT_XFORMS) do
            a, b = fn(a), fn(b)
         end
         return a < b
      end)
      local top = remove(t)
      t[#t] = t[#t] .. ' or ' .. top
      alternatives = t
   end
   return concat(alternatives, ', ')
end


local EXTRAMSG_XFORMS = {
   xform_gsub('any value or nil', 'argument'),
   xform_gsub('#table', 'non-empty table'),
   xform_gsub('#list', 'non-empty list'),
   xform_gsub('functor', 'functable'),
   xform_gsub('(%S+ of) bool([,%s])', '%1 boolean%2'),
   xform_gsub('(%S+ of) func([,%s])', '%1 function%2'),
   xform_gsub('(%S+ of) int([,%s])', '%1 integer%2'),
   xform_gsub('(%S+ of [^,%s]-)s?([,%s])', '%1s%2'),
   xform_gsub('(s, [^,%s]-)s?([,%s])', '%1s%2'),
   xform_gsub('(of .-)s? or ([^,%s]-)s? ', '%1s or %2s '),
}


local function extramsg_mismatch(i, expectedtypes, argu, key)
   local actual, actualtype

   if type(i) ~= 'number' then
      -- Support the old (expectedtypes, actual, key) calling convention.
      expectedtypes, actual, key, argu = i, expectedtypes, argu, nil
      actualtype = _type(actual)
   else
      -- Support the new (i, expectedtypes, argu) convention, which can
      -- diagnose missing arguments properly.
      actual = argu[i]
      if i > argu.n then
         actualtype = 'no value'
      else
         actualtype = _type(actual) or type(actual)
      end
   end

   -- Tidy up actual type for display.
   if actualtype == 'string' and sub(actual, 1, 1) == ':' then
      actualtype = actual
   elseif type(actual) == 'table' then
      if actualtype == 'table' and (getmetatable(actual) or {}).__call ~= nil then
         actualtype = 'functable'
      elseif next(actual) == nil then
         local matchstr = ',' .. concat(expectedtypes, ',') .. ','
         if actualtype == 'table' and matchstr == ',#list,' then
            actualtype = 'empty list'
         elseif actualtype == 'table' or match(matchstr, ',#') then
            actualtype = 'empty ' .. actualtype
         end
      end
   end

   if key then
      actualtype = actualtype .. ' at index ' .. tostring(key)
   end

   -- Tidy up expected types for display.
   local expectedstr = expectedtypes
   if type(expectedtypes) == 'table' then
      local t = {}
      for i, v in ipairs(expectedtypes) do
         if v == 'func' then
            t[i] = 'function'
         elseif v == 'bool' then
            t[i] = 'boolean'
         elseif v == 'int' then
            t[i] = 'integer'
         elseif v == 'any' then
            t[i] = 'any value'
         elseif v == 'file' then
            t[i] = 'FILE*'
         elseif not key then
            t[i] = match(v, '(%S+) of %S+') or v
         else
            t[i] = v
         end
      end
      expectedstr = orconcat(t) .. ' expected'
      for _, fn in ipairs(EXTRAMSG_XFORMS) do
         expectedstr = fn(expectedstr)
      end
   end

   if expectedstr == 'integer expected' and tonumber(actual) then
      if tointeger(actual) == nil then
         return actualtype .. ' has no integer representation'
      end
   end

   return expectedstr .. ', got ' .. actualtype
end


--- Compare *check* against type of *actual*. *check* must be a single type
-- @string expected extended type name expected
-- @param actual object being typechecked
-- @treturn boolean `true` if *actual* is of type *check*, otherwise
--    `false`
local function checktype(expected, actual)
   if expected == 'any' and actual ~= nil then
      return true
   elseif expected == 'file' and io_type(actual) == 'file' then
      return true
   elseif expected == 'functable' or expected == 'callable' or expected == 'functor' then
      if (getmetatable(actual) or {}).__call ~= nil then
         return true
      end
   end

   local actualtype = type(actual)
   if expected == actualtype then
      return true
   elseif expected == 'bool' and actualtype == 'boolean' then
      return true
   elseif expected == '#table' then
      if actualtype == 'table' and next(actual) then
         return true
      end
   elseif expected == 'func' or expected == 'callable' then
      if actualtype == 'function' then
         return true
      end
   elseif expected == 'int' or expected == 'integer' then
      if actualtype == 'number' and actual == floor(actual) then
         return true
      end
   elseif type(expected) == 'string' and sub(expected, 1, 1) == ':' then
      if expected == actual then
         return true
      end
   end

   actualtype = _type(actual)
   if expected == actualtype then
      return true
   elseif expected == 'list' or expected == '#list' then
      if actualtype == 'table' or actualtype == 'List' then
         local n, count = len(actual), 0
         local i = next(actual)
         repeat
            if i ~= nil then
               count = count + 1
            end
            i = next(actual, i)
         until i == nil or count > n
         if count == n and (expected == 'list' or count > 0) then
            return true
         end
      end
   elseif expected == 'object' then
      if actualtype ~= 'table' and type(actual) == 'table' then
         return true
      end
   end

   return false
end


local function typesplit(typespec)
   if type(typespec) == 'string' then
      typespec = split(gsub(typespec, '%s+or%s+', '|'), '%s*|%s*')
   end
   local r, seen, add_nil = {}, {}, false
   for _, v in ipairs(typespec) do
      local m = match(v, '^%?(.+)$')
      if m then
         add_nil, v = true, m
      end
      if not seen[v] then
         r[#r + 1] = v
         seen[v] = true
      end
   end
   if add_nil then
      r[#r + 1] = 'nil'
   end
   return r
end


local function checktypespec(expected, actual)
   expected = typesplit(expected)

   -- Check actual has one of the types from expected
   for _, expect in ipairs(expected) do
      local container, contents = match(expect, '^(%S+) of (%S-)s?$')
      container = container or expect

      -- Does the type of actual check out?
      local ok = checktype(container, actual)

      -- For 'table of things', check all elements are a thing too.
      if ok and contents and type(actual) == 'table' then
         for k, v in pairs(actual) do
            if not checktype(contents, v) then
               return nil, extramsg_mismatch(expected, v, k)
            end
         end
      end
      if ok then
         return true
      end
   end

   return nil, extramsg_mismatch(expected, actual)
end



--[[ ================================== ]]--
--[[ Implementation of function checks. ]]--
--[[ ================================== ]]--


local function extramsg_toomany(bad, expected, actual)
   local s = 'no more than %d %s%s expected, got %d'
   return format(s, expected, bad, expected == 1 and '' or 's', actual)
end


--- Strip trailing ellipsis from final argument if any, storing maximum
-- number of values that can be matched directly in `t.maxvalues`.
-- @tparam table t table to act on
-- @string v element added to *t*, to match against ... suffix
-- @treturn table *t* with ellipsis stripped and maxvalues field set
local function markdots(t, v)
   return (gsub(v, '%.%.%.$', function()
      t.dots = true return ''
   end))
end


--- Calculate permutations of type lists with and without [optionals].
-- @tparam table t a list of expected types by argument position
-- @treturn table set of possible type lists
local function permute(t)
   if t[#t] then
      t[#t] = gsub(t[#t], '%]%.%.%.$', '...]')
   end

   local p = {{}}
   for _, v in ipairs(t) do
      local optional = match(v, '%[(.+)%]')

      if optional == nil then
         -- Append non-optional type-spec to each permutation.
         for b = 1, #p do
            insert(p[b], markdots(p[b], v))
         end
      else
         -- Duplicate all existing permutations, and add optional type-spec
         -- to the unduplicated permutations.
         local o = #p
         for b = 1, o do
            p[b + o] = copy(p[b])
            insert(p[b], markdots(p[b], optional))
         end
      end
   end
   return p
end


local function projectuniq(fkey, tt)
   -- project
   local t = {}
   for _, u in ipairs(tt) do
      t[#t + 1] = u[fkey]
   end

   -- split and remove duplicates
   local r, s = {}, {}
   for _, e in ipairs(t) do
      for _, v in ipairs(typesplit(e)) do
         if s[v] == nil then
            r[#r + 1], s[v] = v, true
         end
      end
   end
   return r
end


local function parsetypes(typespec)
   local r, permutations = {}, permute(typespec)
   for i = 1, #permutations[1] do
      r[i] = projectuniq(i, permutations)
   end
   r.dots = permutations[1].dots
   return r
end



local argcheck = (function()
   if _debug.argcheck then

      return function(name, i, expected, actual, level)
         level = level or 1
         local _, err = checktypespec(expected, actual)
         if err then
            argerror(name, i, err, level + 1)
         end
      end

   else

      return function(...)
         return ...
      end

   end
end)()


local argscheck = (function()
   if _debug.argcheck then

      --- Return index of the first mismatch between types and values, or `nil`.
      -- @tparam table typelist a list of expected types
      -- @tparam table valuelist a table of arguments to compare
      -- @treturn int|nil position of first mismatch in *typelist*
      local function typematch(typelist, valuelist)
         local n = #typelist
         for i = 1, n do   -- normal parameters
            local ok = pcall(argcheck, 'pcall', i, typelist[i], valuelist[i])
            if not ok or i > valuelist.n then
               return i
            end
         end
         for i = n + 1, valuelist.n do -- additional values against final type
            local ok = pcall(argcheck, 'pcall', i, typelist[n], valuelist[i])
            if not ok then
               return i
            end
         end
      end


      --- Diagnose mismatches between *valuelist* and type *permutations*.
      -- @tparam table valuelist list of actual values to be checked
      -- @tparam table argt table of precalculated values and handler functiens
      local function diagnose(valuelist, argt)
         local permutations = argt.permutations
         local bestmismatch, t

         bestmismatch = 0
         for i, typelist in ipairs(permutations) do
            local mismatch = typematch(typelist, valuelist)
            if mismatch == nil then
               bestmismatch, t = nil, nil
               break -- every *valuelist* matched types from this *typelist*
            elseif mismatch > bestmismatch then
               bestmismatch, t = mismatch, permutations[i]
            end
         end

         if bestmismatch ~= nil then
            -- Report an error for all possible types at bestmismatch index.
            local i, expected = bestmismatch, nil
            if t.dots and i > #t then
               expected = typesplit(t[#t])
            else
               expected = projectuniq(i, permutations)
            end

            -- This relies on the `permute()` algorithm leaving the longest
            -- possible permutation(with dots if necessary) at permutations[1].
            local typelist = permutations[1]

            -- For 'container of things', check all elements are a thing too.
            if typelist[i] then
               local contents = match(typelist[i], '^%S+ of (%S-)s?$')
               if contents and type(valuelist[i]) == 'table' then
                  for k, v in pairs(valuelist[i]) do
                     if not checktype(contents, v) then
                        argt.badtype(i, extramsg_mismatch(expected, v, k), 3)
                     end
                  end
               end
            end

            -- Otherwise the argument type itself was mismatched.
            if t.dots or #t >= valuelist.n then
               argt.badtype(i, extramsg_mismatch(i, expected, valuelist), 3)
            end
         end

         local n = valuelist.n
         t = t or permutations[1]
         if t and t.dots == nil and n > #t then
            argt.badtype(#t + 1, extramsg_toomany(argt.bad, #t, n), 3)
         end
      end


      -- Pattern to extract: fname([types]?[, types]*)
      local args_pattern = '^%s*([%w_][%.%:%d%w_]*)%s*%(%s*(.*)%s*%)'

      return function(decl, inner)
         -- Parse 'fname(argtype, argtype, argtype...)'.
         local fname, argtypes = match(decl, args_pattern)
         if argtypes == '' then
            argtypes = {}
         elseif argtypes then
            argtypes = split(argtypes, '%s*,%s*')
         else
            fname = match(decl, '^%s*([%w_][%.%:%d%w_]*)')
         end

         -- Precalculate vtables once to make multiple calls faster.
         local input = {
            bad = 'argument',
            badtype = function(i, extramsg, level)
               level = level or 1
               argerror(fname, i, extramsg, level + 1)
            end,
            permutations = permute(argtypes),
         }

         -- Parse '... => returntype, returntype, returntype...'.
         local output, returntypes = nil, match(decl, '=>%s*(.+)%s*$')
         if returntypes then
            local i, permutations = 0, {}
            for _, group in ipairs(split(returntypes, '%s+or%s+')) do
               returntypes = split(group, ',%s*')
               for _, t in ipairs(permute(returntypes)) do
                  i = i + 1
                  permutations[i] = t
               end
            end

            -- Ensure the longest permutation is first in the list.
            sort(permutations, function(a, b)
               return #a > #b
            end)

            output = {
               bad = 'result',
               badtype = function(i, extramsg, level)
                  level = level or 1
                  resulterror(fname, i, gsub(extramsg, 'argument( expected,)', 'result%1'), level + 1)
               end,
               permutations = permutations,
            }
         end

         local wrap_function = function(my_inner)
            return function(...)
               local argt = pack(...)

               -- Don't check type of self if fname has a ':' in it.
               if find(fname, ':') then
                  remove(argt, 1)
                  argt.n = argt.n - 1
               end

               -- Diagnose bad inputs.
               diagnose(argt, input)

               -- Propagate outer environment to inner function.
               if type(my_inner) == 'table' then
                  setfenv((getmetatable(my_inner) or {}).__call, getfenv(1))
               else
                  setfenv(my_inner, getfenv(1))
               end

               -- Execute.
               local results = pack(my_inner(...))

               -- Diagnose bad outputs.
               if returntypes then
                  diagnose(results, output)
               end

               return unpack(results, 1, results.n)
            end
         end

         if inner then
            return wrap_function(inner)
         else
            return setmetatable({}, {
               __concat = function(_, concat_inner)
                  return wrap_function(concat_inner)
               end
            })
         end
      end

   else

      -- Turn off argument checking if _debug is false, or a table containing
      -- a false valued `argcheck` field.
      return function(_, inner)
         if inner then
            return inner
         else
            return setmetatable({}, {
               __concat = function(_, concat_inner)
                  return concat_inner
               end
            })
         end
      end

   end
end)()


local T = types

return setmetatable({
   --- Add this to any stack frame offsets when argchecks are in force.
   -- @int ARGCHECK_FRAME
   ARGCHECK_FRAME = ARGCHECK_FRAME,

   --- Check the type of an argument against expected types.
   -- Equivalent to luaL_argcheck in the Lua C API.
   --
   -- Call `argerror` if there is a type mismatch.
   --
   -- Argument `actual` must match one of the types from in `expected`, each
   -- of which can be the name of a primitive Lua type, a stdlib object type,
   -- or one of the special options below:
   --
   --    #table    accept any non-empty table
   --    any       accept any non-nil argument type
   --    callable  accept a function or a functable
   --    file      accept an open file object
   --    func      accept a function
   --    function  accept a function
   --    functable accept an object with a __call metamethod
   --    int       accept an integer valued number
   --    list      accept a table where all keys are a contiguous 1-based integer range
   --    #list     accept any non-empty list
   --    object    accept any std.Object derived type
   --    :foo      accept only the exact string ':foo', works for any :-prefixed string
   --
   -- The `:foo` format allows for type-checking of self-documenting
   -- boolean-like constant string parameters predicated on `nil` versus
   -- `:option` instead of `false` versus `true`.   Or you could support
   -- both:
   --
   --    argcheck('table.copy', 2, 'boolean|:nometa|nil', nometa)
   --
   -- A very common pattern is to have a list of possible types including
   -- 'nil' when the argument is optional.   Rather than writing long-hand
   -- as above, prepend a question mark to the list of types and omit the
   -- explicit 'nil' entry:
   --
   --     argcheck('table.copy', 2, '?boolean|:nometa', predicate)
   --
   -- Normally, you should not need to use the `level` parameter, as the
   -- default is to blame the caller of the function using `argcheck` in
   -- error messages; which is almost certainly what you want.
   -- @function argcheck
   -- @string name function to blame in error message
   -- @int i argument number to blame in error message
   -- @string expected specification for acceptable argument types
   -- @param actual argument passed
   -- @int[opt=2] level call stack level to blame for the error
   -- @usage
   --    local function case(with, branches)
   --       argcheck('std.functional.case', 2, '#table', branches)
   --       ...
   argcheck = checktypes(
      'argcheck', T.string, T.integer, T.string, T.accept, opt(T.integer)
   ) .. argcheck,

   --- Raise a bad argument error.
   -- Equivalent to luaL_argerror in the Lua C API. This function does not
   -- return.   The `level` argument behaves just like the core `error`
   -- function.
   -- @function argerror
   -- @string name function to callout in error message
   -- @int i argument number
   -- @string[opt] extramsg additional text to append to message inside parentheses
   -- @int[opt=1] level call stack level to blame for the error
   -- @see resulterror
   -- @see extramsg_mismatch
   -- @usage
   --    local function slurp(file)
   --       local h, err = input_handle(file)
   --       if h == nil then
   --          argerror('std.io.slurp', 1, err, 2)
   --       end
   --       ...
   argerror = checktypes(
      'argerror', T.string, T.integer, T.accept, opt(T.integer)
   ) .. argerror,

   --- Wrap a function definition with argument type and arity checking.
   -- In addition to checking that each argument type matches the corresponding
   -- element in the *types* table with `argcheck`, if the final element of
   -- *types* ends with an ellipsis, remaining unchecked arguments are checked
   -- against that type:
   --
   --     format = argscheck('string.format(string, ?any...)', string.format)
   --
   -- A colon in the function name indicates that the argument type list does
   -- not have a type for `self`:
   --
   --     format = argscheck('string:format(?any...)', string.format)
   --
   -- If an argument can be omitted entirely, then put its type specification
   -- in square brackets:
   --
   --     insert = argscheck('table.insert(table, [int], ?any)', table.insert)
   --
   -- Similarly return types can be checked with the same list syntax as
   -- arguments:
   --
   --     len = argscheck('string.len(string) => int', string.len)
   --
   -- Additionally, variant return type lists can be listed like this:
   --
   --     open = argscheck('io.open(string, ?string) => file or nil, string',
   --                       io.open)
   --
   -- @function argscheck
   -- @string decl function type declaration string
   -- @func inner function to wrap with argument checking
   -- @return function
   -- @usage
   --    local case = argscheck('std.functional.case(?any, #table) => [any...]',
   --       function(with, branches)
   --          ...
   --       end)
   --
   --    -- Alternatively, as an annotation:
   --    local case = argscheck 'std.functional.case(?any, #table) => [any...]' ..
   --    function(with, branches)
   --       ...
   --    end
   argscheck = checktypes(
      'argscheck', T.string, opt(T.callable)
   ) .. argscheck,

   --- Checks the type of *actual* against the *expected* typespec
   -- @function check
   -- @tparam string expected expected typespec
   -- @param actual object being typechecked
   -- @treturn[1] bool `true`, if *actual* matches *expected*
   -- @return[2] `nil`
   -- @treturn[2] string an @{extramsg_mismatch} format error message, otherwise
   -- @usage
   --    --> stdin:2: string or number expected, got empty table
   --    assert(check('string|number', {}))
   check = checktypespec,

   --- Format a type mismatch error.
   -- @function extramsg_mismatch
   -- @int[opt] i index of *argu* to be matched with
   -- @string expected a pipe delimited list of matchable types
   -- @tparam table argu packed table of all arguments
   -- @param[opt] key erroring container element key
   -- @treturn string formatted *extramsg* for this mismatch for @{argerror}
   -- @see argerror
   -- @see resulterror
   -- @usage
   --    if fmt ~= nil and type(fmt) ~= 'string' then
   --       argerror('format', 1, extramsg_mismatch(1, '?string', argu))
   --    end
   extramsg_mismatch = function(i, expected, argu, key)
      if tointeger(i) and type(expected) == 'string' then
         expected = typesplit(expected)
      else
         -- support old (expected, actual, key) calling convention
         i = typesplit(i)
      end
      return extramsg_mismatch(i, expected, argu, key)
   end,

   --- Format a too many things error.
   -- @function extramsg_toomany
   -- @string bad the thing there are too many of
   -- @int expected maximum number of *bad* things expected
   -- @int actual actual number of *bad* things that triggered the error
   -- @see argerror
   -- @see resulterror
   -- @see extramsg_mismatch
   -- @usage
   --    if select('#', ...) > 7 then
   --       argerror('sevenses', 8, extramsg_toomany('argument', 7, select('#', ...)))
   --    end
   extramsg_toomany = extramsg_toomany,

   --- Create an @{ArgCheck} predicate for an optional argument.
   --
   -- This function satisfies the @{ArgCheck} interface in order to be
   -- useful as an argument to @{argscheck} when a particular argument
   -- is optional.
   -- @function opt
   -- @tparam ArgCheck ... type predicate callables
   -- @treturn ArgCheck a new function that calls all passed
   --    predicates, and combines error messages if all fail
   -- @usage
   --    getfenv = argscheck(
   --       'getfenv', opt(types.integer, types.callable)
   --    ) .. getfenv
   opt = opt,

   --- Compact permutation list into a list of valid types at each argument.
   -- Eliminate bracketed types by combining all valid types at each position
   -- for all permutations of *typelist*.
   -- @function parsetypes
   -- @tparam list types a normalized list of type names
   -- @treturn list valid types for each positional parameter
   parsetypes = parsetypes,

   --- Raise a bad result error.
   -- Like @{argerror} for bad results. This function does not
   -- return.   The `level` argument behaves just like the core `error`
   -- function.
   -- @function resulterror
   -- @string name function to callout in error message
   -- @int i result number
   -- @string[opt] extramsg additional text to append to message inside parentheses
   -- @int[opt=1] level call stack level to blame for the error
   -- @usage
   --    local function slurp(file)
   --       ...
   --       if type(result) ~= 'string' then
   --          resulterror('std.io.slurp', 1, err, 2)
   --       end
   resulterror = checktypes(
      'resulterror', T.string, T.integer, T.accept, opt(T.integer)
   ) .. resulterror,

   --- A collection of @{ArgCheck} functions used by `normalize` APIs.
   -- @table types
   -- @tfield ArgCheck accept always succeeds
   -- @tfield ArgCheck callable accept a function or functable
   -- @tfield ArgCheck integer accept integer valued number
   -- @tfield ArgCheck nil accept only `nil`
   -- @tfield ArgCheck table accept any table
   -- @tfield ArgCheck value accept any non-`nil` value
   types = types,

   --- Split a typespec string into a table of normalized type names.
   -- @function typesplit
   -- @tparam string|table either `"?bool|:nometa"` or `{"boolean", ":nometa"}`
   -- @treturn table a new list with duplicates removed and leading '?'s
   --    replaced by a 'nil' element
   typesplit = typesplit,

}, {

   --- Metamethods
   -- @section metamethods

   --- Lazy loading of typecheck modules.
   -- Don't load everything on initial startup, wait until first attempt
   -- to access a submodule, and then load it on demand.
   -- @function __index
   -- @string name submodule name
   -- @treturn table|nil the submodule that was loaded to satisfy the missing
   --    `name`, otherwise `nil` if nothing was found
   -- @usage
   --    local version = require 'typecheck'.version
   __index = function(self, name)
      local ok, t = pcall(require, 'typecheck.' .. name)
      if ok then
         rawset(self, name, t)
         return t
      end
   end,
})


--- Types
-- @section types

--- Signature of an @{argscheck} callable.
-- @function ArgCheck
-- @tparam table argu a packed table (including `n` field) of all arguments
-- @int index into @argu* for argument to action
-- @return[1] nothing, to accept `argu[i]`
-- @treturn[2] string error message, to reject `argu[i]` immediately
-- @treturn[3] string the expected type of `argu[i]`
-- @treturn[3] string a description of rejected `argu[i]`
-- @usage
--    len = argscheck('len', any(types.table, types.string)) .. len

--- Signature of a @{check} type predicate callable.
-- @function Predicate
-- @param x object to action
-- @treturn boolean `true` if *x* is of the expected type, otherwise `false`
-- @treturn[opt] string description of the actual type for error message
