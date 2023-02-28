Conan VTD-API Package
=====================

In order to separate build and runtime dependencies, the VTD API sources files
are put in their own Conan package.

The `vtd/2022.3@cloe-restricted/stable` package must be available. For
instructions on how to build it, see the [README](../vtd/README.md) in the
[vtd](../vtd) directory.

Then just run `make package`.
