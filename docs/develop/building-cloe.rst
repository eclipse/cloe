Building Cloe
=============

If the basic :doc:`requirements <../install>` are fulfilled, and you have
a controller binding, then you essentially only have to do the following,
adjusting as necessary to your project:

1. Clone the `Cloe repository`_::

      git clone git@github.com:eclipse/cloe.git
      cd cloe

2. Install build- and runtime-dependencies.

   See the ``README.md`` file for instructions on dependencies.

3. Install a simulator binding (eg. VTD).

4. Build the project with CMake::

      make all


Build System
------------

If you are developing Cloe itself, it is necessary to understand how the build
system is composed. The following is a description of the files that are
present in the root of the repository and relevant for the building.

``CMakeLists.txt``
    Contains cross-platform instructions for how to build, test, install, and
    package the various parts of Cloe. This file also defines several options,
    which can be toggled on or off during the CMake configuration stage.

    Each component and plugin of Cloe should also have a ``CMakeLists.txt``
    file, so as to keep each file terse. See :doc:`plugin-structure`.

``configure``
    A shell script for conveniently creating common CMake configurations.
    In the end, it is simply a wrapper script around creating a ``build``
    directory and running some variant of ``cmake ..`` in it.
    Please run ``./configure --help`` for the list of accepted options.

``Makefile``
    Contains a set of targets that can be run in this directory. Any task that
    occurs commonly and is a part of development should or can be added to the
    Makefile. This is not only convenient, but also acts as documentation.

    Each component and plugin of Cloe should also have a ``Makefile`` file.
    See :doc:`plugin-structure`.


Testing Cloe
------------

With the help of CMake and the Google Test framework, all unit tests can be
run with::

    make test

This works in the ``build/`` directory as well as from the repository root.
Under the hood it uses CTest to run several other binaries that were compiled
during the standard build stage.


Installing and Packaging Cloe
-----------------------------

With the help of CMake, Cloe can be installed to ``/usr/local`` or whatever
prefix you desire, by running::

    make install

This is NOT recommended however. By installing to ``/usr/local``, a separate
set of Cloe development files are placed in the filesystem which could be used
by dependents of Cloe. Because ``/usr/local/include`` and ``/usr/local/lib``
are automatically included in the include and library search paths, these
could accidentally be used instead of the intended files, leading to obscure
compilation and runtime errors.

Instead, if you want to make Cloe available system-wide, we recommend you
create a package. Again, with the help of CMake, it is possible to automatically
generate Debian packages from the source::

    make package

These can then be installed, either one by one, or all in one go::

    dpkg -i build/*.deb

When actively developing Cloe, it is NOT recommended to have it installed
system-wide, for the same reasons as already explained.


.. _Cloe repository: https://github.com/eclipse/cloe
