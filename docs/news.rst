News
====

.. toctree::
   :hidden:
   :maxdepth: 1

   news/release-0.19.0

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
