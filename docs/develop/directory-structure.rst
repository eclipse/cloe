Project Structure
=================

The top-level directory of the Cloe repository contains build and test
configuration, disclaimers and other files that need to be stored in the top
level for their scope's sake.

Directories
-----------

``build/``

    This directory contains all generated artifacts, especially those created
    by CMake and the build process. This directory is not checked in to the
    repository.

``doc/``

    This directory contains user and developer documentation, mainly in the
    form of a Sphinx project containing reStructuredText (the *.rst* files).
    The Sphinx and Doxygen documentation place their output in the ``_build/``
    subdirectory.

``runtime/``

    This directory contains the core C++ library code and interface headers.
    Any implementation of a Cloe plugin depends on the runtime and will need
    to include the headers and link to the library.

``runtime/engine/``

    This directory contains the source code and resources of the Cloe runtime
    executer, named ``cloe-engine``. This contains the logic of argument parsing,
    configuration, plugin handling, REST API server, main simulation loop,
    and trigger mechanisms.

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

Files
-----

``CMakeLists.txt``

    The main C++ project configuration. This is used by CMake to create a build
    configuration for use with whatever build system you want to use. It is
    highly recommended on Linux, WSL, and MinGW to use ``configure`` and
    ``make`` instead of directly using CMake.

``configure``

    A script to help set up a CMake configuration in the ``build/`` directory.
    Use ``configure -h`` to see what options there are. It is not necessary
    to run this script directly, as ``make`` will do it for you if necessary.

``Makefile``

    Contains all useful development targets. Run ``make`` by itself to get a
    list of applicable targets, sorted by use-case. The targets most commonly
    used are ``clean``, ``all``, and ``smoketest``.
