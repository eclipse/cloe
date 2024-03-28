Lua Editor Integration
======================

In order to have the best user-experience when working with Lua files, it's
important to have a good language server up and running to provide
hinting, auto-completion, and linting.

The Cloe Engine provides definitions for the
[Sumneko Lua Language Server](https://github.com/LuaLS/vscode-lua),
which can be easily integrated in your favorite editor.
For VS Code, install [this extension](https://marketplace.visualstudio.com/items?itemName=sumneko.lua)

The language server may need a configuration file in order to find the
definitions (though this should not be necessary for the Cloe repository.)

Configuration File
------------------

Let us assume that you have a directory `tests` containing Lua files that you
want to include Cloe definitions for.

Place in the `tests` directory or in any directory containing `tests` (such
as the repository root) a file named `.luarc.json` containing the following
content:

```json
{
  "$schema": "https://raw.githubusercontent.com/sumneko/vscode-lua/master/setting/schema.json",
  "workspace.library": ["PATH_CONTAINING_LUA_MODULES"],
  "runtime.version": "Lua 5.3",
  "completion.displayContext": 1,
  "diagnostics.globals": [],
  "hint.enable": true
}
```

Until we develop a plugin for Sumneko containing all the definitions, you need
to tell Sumneko where to find them by hand, where `PATH_CONTAINING_LUA_MODULES`
is above.

One approach is to make a symlink to the source Lua files in your own
repository and set the workspace library to the symlink:

```sh
git clone https://github.com/eclipse/cloe ~/cloe
ln -s ~/cloe/engine/lua meta
sed -e 's/PATH_CONTAINING_LUA_MODULES/meta/' -i .luarc.json
```

If you are not committing the `.luarc.json` file, then you can also just
specify the absolute path.
