News
====

..
    TODO(release) // Write release news article

    1. Write a news entry about the new release, in the `news/`
       folder and named `release-X.Y.Z`. See the previous release
       and follow the same pattern.

       These blog entries should cover the truly interesting
       changes and also help a user migrate from an older version
       if they need to make any changes.

    2. Add it to the toctree below and write or copy a TLDR from
       the article.

    3. Keep the written TLDRs from the last few releases in this
       document and remove the old ones. Keep at least three.


.. toctree::
   :hidden:
   :maxdepth: 1

   news/release-0.24.0
   news/release-0.23.0
   news/release-0.22.0
   news/release-0.21.0
   news/release-0.20.0
   news/release-0.19.0


:doc:`Version 0.24.0 Release <news/release-0.24.0>`
---------------------------------------------------

This is a small release that contains two changes:

Firstly, the ``cloe-launch`` interface is significantly improved and simplified:

- Launch profiles are no longer "managed"; instead use Conan recipes directly.
- Arguments to Conan can be passed as-is after specifying the Conan recipe.
- Common Conan errors are summarized for the user.
- New command ``config`` lets you manage the launcher configuration.
- New command ``deploy`` lets you create a binary deployment of a Conan recipe.

Secondly, this release includes the new ``frustum_culling`` plugin.

Read more about these changes and more :doc:`here <news/release-0.24.0>`.


:doc:`Version 0.23.0 Release <news/release-0.23.0>`
---------------------------------------------------

This version brings full C++17 support to Fable, with the following highlights:

- Schema support for `std::filesystem::path`
- Schema support for `std::optional`
- Schema support for `std::array`

Boost is now an optional dependency, and requires inclusion of an extra header
to use.

The old `Array` schema, which was for `std::vector`, has been renamed to
`Vector`, and its header moved to `fable/schema/vector.hpp`.

Read all about these and the other many changes :doc:`here <news/release-0.23.0>`.

:doc:`Version 0.22.0 Release <news/release-0.22.0>`
---------------------------------------------------

This version includes the initial release of the ESMini simulator
plugin binding.

The cloe-osi library is introduced for a re-usable mechanism of
handling and converting Open Simulation Interface data.

The clothoid-fit component plugin for lane boundaries makes its
entrance.

Finally, compile-time improvements for the fable library radically
speed up the compilation of code that use the `make_schema` functions.

Read all about it :doc:`here <news/release-0.22.0>`.

:doc:`Version 0.21.0 Release <news/release-0.21.0>`
---------------------------------------------------

This version includes fixes and other improvements and represents
a continuation of the previous releases before breaking changes
are introduced in the next few releases.

Oat++ replaces the cppnetlib library, which allows us to remove
any constraints regarding the version of Boost that can be used.

The launcher now provides a better error message when the engine
cannot be located and issues with the Zsh prompt are fixed.

The web UI can insert triggers again.

Two significant bugs in the fable library have been fixed.

Finally, many small development and tooling changes are included
in the release, but should not be relevant for end-users.

Read all about it :doc:`here <news/release-0.21.0>`.

:doc:`Version 0.20.0 Release <news/release-0.20.0>`
---------------------------------------------------

This version includes several improvements to the simulator plugin for
VTD and an update to the stack file format, among other smaller improvements.

The `vtd` simulator binding now has support for VTD 2022.3,
OpenScenario files, and the `scp` action to send SCP messages from
triggers.

You can now integrate custom vehicle dynamics models as Cloe
plugins.

The stackfile schema format has been updated to support minor versions.

Triggers can have an `optional: true` field set to make them not fail
when the action or event does not exist.

The fable library has been refactored and may require you to include
`fable/schema/number_impl.hpp` or `fable/schema.hpp` for your code to keep
compiling. Compile performance should be better though.

The tooling in the project has been updated for better compatibility with
Conan 2.0 and packages that use more recent Conan features.

Read all about it :doc:`here <news/release-0.20.0>`.

:doc:`Version 0.19.0 Release <news/release-0.19.0>`
---------------------------------------------------

This version contains some breaking changes in how the project is built.
It also contains improvements across the board.

The Makefiles have been significantly refactored to simplify the build
experience and reduce maintenance effort.

BATS tests use direct `cloe-engine` commands and are run within
a `cloe-launch shell`. The profile used for tests is no longer `conantest.py`,
rather multiple test configurations are stored in the `tests/` directories
themselves.

Optional packages are put in `optional/` in their own subdirectory, along
with all their own vendor packages.

The `cloe-launch` tool now has a subcommand `prepare` that can be used
for building the runtime configuration of a conanfile.py "profile" that should
be used with `cloe-launch`.

The `cloe-engine` package can be built with the most recent versions of
Boost by disabling the `server` feature. The binary has gained several options
and can read options from environment variables. These are aimed towards making
testing orthogonal and easily reproducible. JSON Stackfiles can now contain
comments.

Read all about it :doc:`here <news/release-0.19.0>`.
