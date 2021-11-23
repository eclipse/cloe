Plugin Structure
================

In order to be extensible, Cloe is designed to load certain functionality as
a plugins during runtime.

In the main Cloe repository, plugins are stored in the ``plugins`` directory.
External plugins may interoperate with Cloe, but are maintained in their own
repository. Submodules including Cloe into a plugin should *not* be used.
Each plugin shall have the following structure::

    bin/
    cmake/
    ci/
    contrib/
    doc/
    docker/
    include/
    src/
    tests/
    ui/
    conanfile.py
    CMakeLists.txt
    Makefile
    README.md

Each of the elements should be used as is necessary for the plugin.

``bin/``

    Executables placed in this directory are considered useful enough to
    be ultimately deployed into a common binary directory.

``cmake/``

    If you require special CMake modules or scripts, place them in this
    directory. They will not be automatically loaded, you will need to add
    a line like the following to your ``CMakeLists.txt``::

        list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

``ci/``

    CI related files are placed in this directory.
    If only one CI file is required, it is also acceptable to place it in the
    root, with a descriptive name such as ``Jenkinsfile``.

    Ideally, the ``Makefile`` should be extended with targets that implement
    and describe the purpose of the CI files, and let developers test them
    on their machines.

``contrib/``

    For everything that doesn't fit in the other categories described here,
    such as shell completion scripts, project files, etc.

    .. warning::
        Deprecated. Historically the ``contrib`` directory contained files that
        were not maintained by the owner of the repository, which is something
        that we do not want to imply.

``doc/``

    Any documentation that goes beyond the ``README`` should be organized
    in this directory.

``docker/``

    Docker related files are placed in this directory.
    If only one Dockerfile is required, it is also acceptable to place a single
    ``Dockerfile`` in the root. It is assumed that this will create an image
    of the plugin.

    The ``Makefile`` should be extended with targets that implement and describe
    the purpose of the Dockerfile, e.g. ``build-image``, ``push-image``.

``include/``

    Generally, this is where the interface files are. These will be made
    available to the rest of the programs normally, or installed to the system
    when the project is installed.
    I
``src/``

    Generally, the main source code is stored in this directory.

``ui/``

    If the project exposes React.js components that should be integrated in the
    Cloe UI, then they should be stored in this directory.

``conanfile.py``

    Conan recipe for the plugin.

``CMakeLists.txt``

    Every project should be addable to a CMake project via either the
    ``add_subdirectory`` or ``find_package`` commands.

``Makefile``

    The ``Makefile`` shall include the ``Makefile.package`` from the repository
    root directory. The default package targets can be extended and the default
    options can be changed as needed.

``README``

    Every plugin should have a README file, in some standard plaintext format
    (``.md``, ``.rst``, ``.txt``).
