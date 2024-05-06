# Version 0.24.0 Release

This release primarily refactors and improves the `cloe-launch` tool,
with one major breaking change.

In addition, it includes the new `frustum_culling` plugin.

The following sections are about the changes to the launcher,
starting with the breaking change and how to migrate.

## Remove launcher profiles and change Conan argument pass-thru

For a while, `cloe-launch` has supported launcher profiles, which were Conan
recipes (e.g. `conanfile.txt`) that were installed to a launcher profiles
directory and managed by `cloe-launch`. You could then refer to these from
anywhere with `-p PROFILE_NAME` argument.

In practice however, the `cloe-launch profile` system was under-used,
confusing, and limited. To name just a few issues:

- Profile names needed to end with `.py` if they were Python recipes.
- Python recipes often could not be used at all because they referred to local
  files.
- Passing arguments to Conan was limited and verbose.
- Different commands supported profiles differently, leading to an
  un-orthogonal interface.
- Handling `-p` or `-P`, one of which was required, led to complex code and
  error messages, confusing users.
- Launcher "profiles" uses the same word as Conan "profiles", but mean two
  very different things.

This release radically simplifies the launcher interface by doing away with the
launcher profile system and opting for a clearer, leaner, and more flexible
interface to boot.

This is a significant breaking change, because it fundamentally changes how
`cloe-launch` parses arguments. They now have the form:

    cloe-launch [OPTS] COMMAND [CMD_OPTS] CONANFILE [CONAN_INSTALL_ARGS] [-- ARGS]

For commands such as `shell` and `exec` it's now necessary to use `--`
to pass arguments to them. It is also no longer possible to provide
options to the cloe-launch command after specifying the conanfile.
These now go to conan.

**Migration:**

Find use of profiles by searching for instances of `cloe-launch` and `-P` and `-p`.
Make sure you pass existing Conan recipes to `cloe-launch` as a positional
argument, and that all arguments to cloe-launch have been specified before
the recipe is specified.

If you used `-p` with the launcher profiles before, you will need to find a new
solution, such as by using symlinks or copies of recipes, or creating aliases
for your favorite configurations.

Previously, Conan arguments needed to be passed as arguments to cloe-launch
arguments, such as:

    cloe-launch shell -P conanfile.py -o:o cloe-engine:with_server=False
    cloe-launch shell -o --build=missing -o -o=cloe-engine:with_server=False -P conanfile.py

Now, these are provided as Conan arguments simply after the Conan file:

    cloe-launch shell conanfile.py --build=missing -o cloe-engine:with_server=False

When using `cloe-launch exec`, you can specify the arguments to `cloe-engine`
after a *mandatory* `--` argument:

    cloe-launch exec conanfile.py -- -l debug run test.json

To migrate your own use, search for `cloe-launch` and `-o` or `-o:o` or `-o:s` and
replace these with the updated form.

## Add `config` sub-command

This command lets you view the launcher configuration:

    cloe-launch config

As well as edit it:

    cloe-launch config --edit

If the configuration has not been created yet, it is recommended to
create it first from the default configuration:

    cloe-launch config --write --edit

## Add `deploy` sub-command

This allows you to deploy a configuration to a directory:

    cloe-launch -v deploy -P engine/tests/conanfile_with_server.py /usr/local

This deployment should work without any further setup required.

The `patchelf` tool needs to be installed for RPATH adjustment to work.

An uninstaller is created at ~/.cache/cloe/launcher/UUID/uinstall.sh
where UUID is unique and determined from the configuration.

## Add `--ignore-plugin-setups` option

This options lets you prevent Python configuration files from being loaded
from the directories specified in `CLOE_PLUGIN_PATH`.

This is especially useful if you have added `.` to `CLOE_PLUGIN_PATH`.

## Improve error messages

Conan error output can be hard to parse. There are several common errors that
are now "caught" by the launcher and re-formatted in a way that it's clearer
to users what the error is and what they can do about it.

## Further launcher fixes

- Unpin `click` and `toml` versions.
- Fix `cloe-launch prepare` not showing output.
- Prevent `.` from in-advertently being added to `CLOE_PLUGIN_PATH`.
- Show conan command being executed in verbose mode.
