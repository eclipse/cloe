Introduction to Lua
===================

From version 0.22 Cloe will support the use of Lua for configuring and
scripting simulations. This is a major improvement in usability but does
require some getting used to.

[Lua](https://www.lua.org) is a simple language made for embedding in existing
applications and is very widely used in the industry where user extensibility
and scripting is important. It can be [learned](https://learnxinyminutes.com/docs/lua)
quickly. It is also flexible, which allows us to provide an ergonomic
interface to scripting Cloe.

Setting up Lua
--------------

Lua is embedded in the `cloe-engine`, so if you can run `cloe-engine`, you can
use Lua as an input for `cloe-engine run`, and you can also start an interactive
REPL shell with `cloe-engine shell`:

    $ cloe-engine shell
    Cloe 0.22.0 Lua interactive shell
    Press [Ctrl+D] or [Ctrl+C] to exit.
    > print "hello world!"
    hello world!
    >

### System Lua

You can also install Lua as a system program, such as with Apt:

    sudo apt install lua5.4

The `lua5.3` package is not a development dependency of Cloe, but it does
provide a very simple `lua` binary that you can run to get a Lua REPL
independently of `cloe-engine`. Unfortunately, because `cloe-engine` exports
modules containing C++ types and functions, `lua` by itself isn't as useful
for most use-cases pertaining to Cloe.

:::{note}
In `cloe-engine` we embed Lua 5.4, but on Ubuntu versions older than 22.04 the
latest system version available is `lua5.3`. For the most part, the differences
are not important to us.
:::

### Lua Rocks

More useful to us than a system Lua REPL is the [LuaRocks](https://luarocks.org/)
package manager. This allows us to easily install and manage third-party Lua
libraries. These are then available to Cloe itself.

This can be installed on your system with Apt:

    sudo apt install luarocks

And then packages, called *rocks*, can be installed with the `luarocks` program:

    luarocks install luaposix

See the LuaRocks website for a list of available rocks and also for more
information on how to use LuaRocks.

Suggested Exercises
-------------------

1. Install the latest version of [Lua](https://www.lua.org) on your system.

2. Read one of these introductions to Lua:
   - [Learn Lua in 15 Minutes](https://learnxinyminutes.com/docs/lua/)
   - [Programming in Lua](https://www.lua.org/pil/contents.html)
   - [Lua-Users Wiki](http://lua-users.org/wiki/LearningLua)
   - [Codecademy Course](https://www.codecademy.com/learn/learn-lua)

3. Launch the Cloe REPL and run the following snippet:
   ```lua
   cloe.describe(cloe)
   ```

4. Install the [LuaRocks](https://luarocks.org/) package manager.

5. Install the `luaposix` rock.
