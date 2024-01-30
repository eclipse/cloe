# Version 0.21.0 Release

This version comprises various fixes and improvements but no major changes,
except that C++17 is now the minimum required C++ standard and the
cppnetlib dependency has been replaced with Oat++.

The next few releases will contain breaking changes, so this one acts as
a small incremental step before these are released. In particular, this release
also introduces the `version.hpp` header to the `fable` and `cloe-runtime`
packages, which enables developers to more easily support multiple versions at
the same time.

For the entire changelog, see the [Git commit history](https://github.com/eclipse/cloe/compare/v0.20.0...v0.21.0).

## Launcher

Changed:

- Use `VirtualRunEnv` Conan generator, which is the current recommended approach
  and should be compatible with Conan 2.0.

Improved:

- Provide better error messages when cloe-engine is not found.

  The Python stack trace is suppressed, since it's not useful, and instead a
  message is printed telling you what could be wrong:

      Error: cannot locate cloe-engine exectuable!
￼     Note:
￼       This problem usually stems from one of two common errors:
￼       - The conanfile for cloe-launch does not require cloe-engine.
￼       - The cloe-engine package or binary has not been built / is corrupted.
￼       However, unconvential or unsupported package configuration may also trigger this.

Fixed:

- Fix prompt in Zsh when in a cloe-launch shell.

## Engine

New:

- Add the `loop` event that triggers every single cycle, including in the pause state.

Improved:

- Replace libcppnetlib with Oat++ as webserver backend.

  This means that we are no longer limited in which Boost versions we support.
  (Previously, with the `server` feature enable, the Boost version had to be
  between 1.65 and 1.69.)

## Web UI

Fixed:

- Fix failure uploading json.gz to the UI
- Fix certain actions not applying when trying to trigger the HMI

## Fable Library

Fixed:

- Instantiate missing `Number<signed char>`.

  It is no longer necessary to include `fable/schema/number_impl.hpp` when using this
  numerical type.
- Fix `FromConfable` schemas occassionaly causing a segfault.

  Error can be categorized as a use-after-free.

## Plugins

New:

- Add new schema to minimator plugin to specify objects and properties.

Fixed:

- Fix segfault when gndtruth-extractor plugin had an error opening the output file.
- Fix use of uninitialized `mount_pose` variable.

## Development

New:

- Add `version.hpp` header to `fable` library.

  This can be used to make compile-time decisions for greater compatibility with
  various versions of the library.
- Add `version.hpp` header to `cloe-runtime` library.

  This can be used to make compile-time decisions for greater compatibility with
  various versions of the library.
- Add Clang Tidy configuration file

Improved:

- Speed up builds by changing default build type from `RelWithDebInfo` to `Debug`.
- Bundle licenses of dependencies with `cloe-engine` Conan package.

Changed:

- Change `configure` target in `Makefile.package` to link `compile_commands.json` automatically
- Remove `conan-select` target from `Makefile.all`
- Remove `conan` target from `Makefile.package`
- Require C++17 for compiling the project.

  The oldest supported version of Ubuntu has a new enough compiler to support this
  change, so we don't expect anyone to have any problems with this.
- Update dependencies to latest stable versions.

  They are now also pinned to specific versions, since this results in a much more
  stable experience with Conan. This is also how it is done in the Conan-Center-Index.
- Remove all ifndef-define-endif header guards. Use `#pragma once` instead.

  Our dependencies already require support for `#pragma once`, which is effectively
  universal anyway.
- Remove Ubuntu 18.04 from automated builds. This means it is no longer officially
  supported.
- Improve Conan 2.0 support across the board for package recipes.

Fixed:

- Fix `setup-conan` target in `Makefile.setup` to install Conan version 1.x
- Fix recipes to respect Conan `--build`, `--test`, `--configure`, and `--install` arguments
- Test configurations depend on any compatible version of cloe-launch-profile
- Don't fail test when *.so glob does not match anything
