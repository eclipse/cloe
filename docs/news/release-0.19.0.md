# Version 0.19.0 Release

This version contains some breaking changes in how the project is built.

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
