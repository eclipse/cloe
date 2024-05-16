Lua Initialization
------------------

When a Lua file or script is loaded, the Cloe engine provides a preloaded
`cloe` table with a large API. This API is defined in part through a Lua
runtime, and in part from the C++ engine itself.

The following operations occur when the engine runs a simulation defined
by a Lua file: `cloe-engine run simulation.lua`

1. Read options from the command line and environment:

   - Lua package path (`--lua-path`, `CLOE_LUA_PATH`)
   - Disable system packages (`--no-system-lua`)
   - Enable LRDB Lua debugger (`--debug-lua`)
   - Cloe plugins (`--plugin-path`, `CLOE_PLUGIN_PATH`)

2. Initialize Cloe Stack

   - Load plugins found in plugin path

3. Initialize Lua

   - Set lua package path
   - Load built-in Lua base libraries (e.g. `os`, `string`)
   - Expose Cloe API via `cloe` Lua table
   - Load Cloe Lua runtime (located in the package `lib/cloe/lua` directory)

4. Start LRDB Lua debugger (Optional)

5. Source input files

   - Files ending with `.lua` are merged as Lua
   - Other files are read as JSON

6. Start simulation

   - Schedule triggers pending from the Lua script
