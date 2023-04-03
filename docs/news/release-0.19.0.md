# Version 0.19.0 Release

This version contains some breaking changes in how the project is built.
It also contains improvements across the board. This post highlights some of
the more interesting changes.

For the entire changelog, see the Git commit history.

## Makefile Refactoring

TLDR: The Makefiles have been significantly refactored to simplify
the build experience and reduce maintenance effort.

New:
- Add coloring to output of `help` target.
- Add `setup-conan` target to install Conan profiles for Cloe.
- Add `export-cli` target to export `cloe-launch-profile` Conan recipe.

Changed:
- Rename `setup` target to `setup-git`.
- Rename `package-auto` target to `package`, which it replaces.
- Remove `package-all` target.
- Remove `package-outdated` target.
- Remove `USE_NPROC` option. This was not reliable.
- Remove `WITH_ENGINE` option. This was never used.
- Remove `WITH_VTD` option. This is a variant that is hard to maintain;
  it now resides in its entirety in a separate directory.
- Remove `BUILD_TESTS` option. These should always be run.
- Change `help` target to be created with macro functions, these reside in
  `Makefile.help`, which is now a dependency for all other Makefiles.

There were too many different ways to build Cloe packages with the Makefile,
so we removed ones that were seldom used or did not work reliably.
The problem with the `package` targets and their variants is that
they produced packages that were only valid within their own parameters,
not those required to actually run system tests. Conan will build packages,
but when trying to use `cloe-launch` with a profile, it will complain that
the desired package doesn't exist (because it doesn't, not in the correct
ABI configuration).

For example, `cloe-engine` requires `boost/[>=1.65.1,<1.70]` if built with the
server component, otherwise `boost/[>=1.65.1]`. Conan will pick the newest
Boost version available in the cache that is compatible, and if not already
cached, the newest version from the first remote. So, with the server: `boost/1.69.0`,
without the server: `boost/1.75`. But only on my machine, and only today.
Now if you include a plugin that requires ROS, then you need to make sure that
the Boost version is identical, otherwise you will get segfaults or undefined
behavior when you bring everything together. But Conan doesn't know that
when compiling `cloe-engine`, unless you use a lockfile or build it as a
dependency of a Conan recipe that contains all relevant dependencies.

The current correct way to build Cloe is:

    make export-vendor export
    make LOCKFILE_SOURCE=test/profile_default.py package

This will force build all packages that come from the Cloe repo. If you just
want to build missing or outdated packages, you can use `cloe-launch`:

    make export-vendor export
    cloe-launch prepare -P test/profile_default.py

In each of these cases, you are using the Conanfile `test/profile_default.py` as
the lockfile source and final configuration. You can build all required
configurations for all packages in one go with:

    make export-vendor export smoketest-deps

## Tests Refactoring

TLDR: BATS tests use direct `cloe-engine` commands and are run within
a `cloe-launch shell`. The profile used for tests is no longer `conantest.py`,
rather multiple test configurations are stored in the `tests/` directories
themselves.

In this release we support testing multiple configurations, such as with
`cloe-engine:server=True` and `cloe-engine:server=False`. This requires
running `cloe-engine` in different environments, in different build
configurations. This also allows you to build only what you use for a test.
If you don't need `vtd`, then it's not in the test Conan recipe.

Each of these configurations needs to be prepared and built however.
This can be done with the `smoketest-deps` dependency, which is
available in every package:

    make smoketest-deps

And following that, the system tests can be run with:

    make smoketest

This will find every `profile_*.py` file in the `tests/` directory,
and run all BATS tests in the `tests/` directory with that configuration.
With a single profile, running the tests now looks like this:

    cloe-launch shell -P tests/profile_with_server.py -- -c "bats tests"

This also means that `cloe-launch` is no longer called within BATS, as it used
to be. BATS tests are simplified to the point where the commands in the BATS
tests could also be directly run on the command line. This speeds up tests
significantly, and makes them easier to debug, since all you need to do to get
identical runtime conditions is run `cloe-launch shell` with the same runtime
profile.

The `smoketest` target does essentially the same as the above command, except
for each profile it finds and with some workarounds for older BATS versions.

## Optional Packages

TLDR: Optional packages are put in `optional/` in their own subdirectory, along
with all their own vendor packages.

Up until now, optional packages like `vtd` were included by setting `WITH_VTD=1`
variable when running `make package` or `make smoketest`. This does not scale
well, especially as more simulators are added.

Hence, `optional/vtd` now contains its own vendor packages, tests and test
configurations, and Docker build files. It serves as a model for how an
independent plugin repository can be structured.

The workflow to work on optional packages is as follows:

1. First create or export the standard Cloe Conan packages.
2. Work on the optional package.

If you want to build the Docker image, this also bases on the Docker
image from the standard Cloe which needs to be built first.

## CLI Improvements

TLDR: The `cloe-launch` tool now has a subcommand `prepare` that can be used
for building the runtime configuration of a conanfile.py "profile" that should
be used with `cloe-launch`.

New:
- Add `prepare` subcommand to `cloe-launch`
- Add `cloe-launch-profile` conan package to be used as a conan base class
- Add `[cloe-shell]` prefix to prompt after using `cloe-launch shell`
- Add `--version` flag to `cloe-launch`

The commands `activate`, `exec`, and `shell` that `cloe-launch` provides may run
Conan commands in order to provide activation scripts, launch the Cloe engine,
or spawn a new shell environment, respectively. The output from these Conan
executions are collected and printed as a single logging statement, or if an
error occurs. During this time, no output is printed. This can appear as if
`cloe-launch` is not progressing, and should be terminated. The `prepare`
command just prepares the virtual runtime environment and redirects Conan output
directly to stdout and stderr, providing realtime feedback. It is highly
recommended to use this command and follow it up with other commands that use
the `--cache` option.

When running `cloe-launch` with any of the commands, we provide a Conanfile
to be used as configuration with the `--profile-path` flag. Previously, it was
difficult to inject environment variables in the runtime environment that
`cloe-launch` generates. If the Conanfile depends on the `cloe-launch-profile`
and uses the Python base class provided from that package, it becomes easier
to inject environment variables:

```python
class CloeTest(ConanFile):
    python_requires = "cloe-launch-profile/[~=0.20.0]@cloe/develop"
    python_requires_extend = "cloe-launch-profile.Base"

    @property
    def cloe_launch_env(self):
        return {
            "CLOE_ENGINE_WITH_SERVER": "0",
            "CLOE_LOG_LEVEL": "debug",
            "CLOE_STRICT_MODE": "1",
            "CLOE_WRITE_OUTPUT": "0",
            "CLOE_ROOT": Path(self.recipe_folder) / "../..",
        }

    ...
```

This is used in all tests in the repository to set `cloe-engine` options and
allows us to make BATS tests fully orthogonal with those run in a cloe-launch
shell.

Finally, in the interest of making it more obvious to users when they are in
a `cloe-launch` shell, we now prepend the string `[cloe-shell]` to the prompt.
The `--version` flag also makes checking which `cloe-launch` version you have
possible.

## Engine Improvements

TLDR: The `cloe-engine` package can be built with the most recent versions of
Boost by disabling the `server` feature. The binary has gained several options
and can read options from environment variables. These are aimed towards making
testing orthogonal and easily reproducible. JSON Stackfiles can now contain
comments.

New:
- The engine supports `//`-style comments in all JSON files
- Server component can be compiled out with Conan package option `engine:server=False`
- Add `--strict` flag as an alias for `--no-system-plugins`, `--no-system-confs`, and `--require-success`
- Add `--secure` flag as an alias for `--strict`, `--no-hooks`, and `--no-interpolate`
- Add `--no-` variants for several options
- Read select option defaults from environment variables:
    - `CLOE_LOG_LEVEL` for `--level`
    - `CLOE_REQUIRE_SUCCESS` for `--require-success`
    - `CLOE_SIMULATION_UUID` for `--uuid`
    - `CLOE_WRITE_OUTPUT` for `--write-output`
    - `CLOE_STRICT_MODE` for `--strict`
    - `CLOE_SECURE_MODE` for `--secure`
    - `CLOE_PLUGIN_PATH`

The original JSON specification does not allow for any comments. Recent
alternative standards have arose to address this, such as
[JSONC](https://changelog.com/news/jsonc-is-a-superset-of-json-which-supports-comments-6LwR)
and [JSON5](https://json5.org/).
This release provides support for JSON files with comments, as specified
in JSONC and as supported by
[nlohmann-json](https://github.com/nlohmann/json#comments-in-json).
It is enabled by default in `cloe-engine`, but can be disabled by setting the
`fable:allow_comments=False` option when compiling the `cloe-engine` package.

Our current embedded server implementation builds on Boost internals that have
changed in 1.70, such that it is not possible to use a newer Boost version and
the embedded web server at the same time. By making the server an optional
component, we make it possible to use newer versions of Boost until we migrate
to a different server implementation. In your Conanfile where you include the
`engine` package, set the `server` option in the `default_options` attribute,
like so:

```python
class CloeTest(ConanFile):
    ...
    default_options = {
        "cloe-engine:server": False,
    }
    ...

    def requirements(self):
        self.requires(f"cloe-engine/0.20.0@cloe/develop")
        ...
```

This option can also be specified in your Conan profile, system-wide, as well as
on the command line when running Conan commands. See the Conan documentation for
more details on this.

Several options have been added to `cloe-engine` in order to better support
our changes to testing, as detailed in the previous sections. Since options
provided by the environment are set as the default, we only have a way to
override them if we also provide negations of options. This release provides
those negations, such as `--no-strict`.
