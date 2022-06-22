Project Structure
=================

The top-level directory of the Cloe repository contains build and test
configuration, disclaimers and other files that need to be stored in the top
level for their scope's sake.

Directories
-----------

``build/``

    Since Cloe is composed of multiple Conan packages that are either built in
    the Conan cache or in the respective package subdirectories, the top-level
    ``build/`` directory only contains generated documentation artifacts. This
    directory is not checked in to the repository.

``cli/``

    This directory contains the source code of the ``cloe-launch`` tool. Refer
    to :doc:`Using Cloe Launch<../usage/using-cloe-launch>` for details on its capabilities.

``dist/``

    This directory contains scripts and tooling to facilitate the distribution
    of Cloe. Dockerfiles are provided to build the Cloe Conan packages on
    several operating systems and to upload them to a Conan remote.

``doc/``

    This directory contains user and developer documentation, mainly in the
    form of a Sphinx project containing reStructuredText (the *.rst* files).
    The Sphinx and Doxygen documentation place their output in the ``build/``
    directory.

``engine/``

    This directory contains the source code and resources of the Cloe runtime
    executer, named ``cloe-engine``. This contains the logic of argument parsing,
    configuration, plugin handling, REST API server, main simulation loop,
    and trigger mechanisms.

``fable/``

    This directory contains the fable library, which provides functionalities
    for serialization/deserialization to/from JSON structures. It is used
    to implement the configuration of Cloe and its plugins, as well as for
    Cloe's JSON API.


``models/``

    This directory contains the definition of Cloe's data model and its
    interfaces.

``oak/``

    This directory contains the web server.

``plugins/``

    This directory contains all Cloe plugins that are maintained by the Cloe
    development team and are ready-for-use.

    In order to keep the runtime and the engine as lean as possible, Cloe is
    extended with plugins. These can be simulator bindings, controller bindings,
    or components.

    Each plugin has its own directory structure and can be maintained
    separately from the rest of the Cloe code. This makes it possible to
    separate each plugin from the core Cloe library with minimal effort.

    See also Section :doc:`plugin-structure`.

``runtime/``

    This directory contains the core C++ library code and interface headers.
    Any implementation of a Cloe plugin depends on the runtime and will need
    to include the headers and link to the library.


``tests/``

    This directory contains system tests for Cloe, mainly in the form of Cloe
    Stackfiles and BATS scripts. These can also be used as templates for those
    just getting started with Cloe.

    .. note::
       Unit tests are not contained here, but rather in the source
       directories of the respective C++ and Python files.

``tools/``

    This directory contains scripts and resources for improving the development
    workflow.

``ui/``

    This directory contains Cloe's Web UI. For more details, refer to the
    :doc:`Running the Cloe Web UI<../usage/running-cloe-webui>` section.


``vendor``

    This directory contains Conan recipes to build third-party libraries
    that are used by Cloe, but are not available as Conan packages on public
    servers.

Files
-----

``conanfile.py``

    Conan recipe for the ``cloe`` meta-package
    (s. :doc:`Understanding Cloe Packages<../usage/understanding-cloe-packages>`).

``LICENSE``

    Terms and conditions for use, reproduction and distribution of Cloe.

``Makefile``

    Make target definitions for the Cloe project. Both ``Makefile.setup`` and
    ``Makefile.all`` are merged in by inclusion.

``Makefile.package``

    Make target definitions for building, installing and testing a Conan
    package. It is used for all Cloe packages, including the ``cloe``
    meta-package (s. ``Makefile.all``).

``VERSION``

    By default, the Cloe version is determined using ``git describe`` (try the
    command). Alternatively, the Cloe version can be directly specified by creating
    the ``VERSION`` file, which must contain the version string only
    (e.g. ``0.18.0-nightly``). You can verify that this version is used by
    executing::

        make -f Makefile.package info-version

    The ``VERSION`` file is not checked in to the repository.

    Generally we want packages to be immutable. So if I refer to
    ``cloe/0.18.0-rc1``, I expect to always get the package built from the
    code checked out from the ``v0.18.0-rc1`` tag.
    On the other hand, during development it can lead to a lot of unnecessary
    re-compilation, especially when structuring commits and rebasing. To this
    end, we can set ``VERSION`` to contain a version with a suffix that is
    understood to be transient, such as ``0.18.0-nightly`` or the name of the
    branch in question, such as ``master``.
