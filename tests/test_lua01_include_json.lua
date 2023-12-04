-- Simplest configuration just loads an existing stackfile.
--

-- The `cloe` table is loaded by default within cloe-engine.
--
-- This will do nothing in cloe-engine, but it will give us
-- completion when using a Lua LSP.
local cloe = require("cloe")

-- cloe.has_feature() lets us check whether we have a certain
-- version or feature of cloe.
assert(cloe.has_feature("cloe-0.21"), "cloe is not recent enough")

-- cloe.load_stackfile() lets us load a stackfile. This may
-- be dropped or shimmed in the future.
cloe.require_feature("cloe-stackfile-4")
cloe.load_stackfile("config_nop_smoketest.json")
