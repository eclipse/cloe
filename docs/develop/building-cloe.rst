Building Cloe
=============

.. note::
  Before you follow any of the steps described below, please make sure that you
  complete all steps described on the :doc:`Installation<../install>` page.
  Please also read the sections on
  :doc:`Understanding Cloe Packages<../usage/understanding-cloe-packages>` first.

Suppose you intend to modify the content of one of Cloe's multiple Conan
packages. Below you can find some hints on how to build and test your package of
interest without exporting it to the `Conan local cache`_ every time.

Build System
------------

If you are developing Cloe itself, it is necessary to understand how the build
system is composed. In essence, we are using GNU Make as a wrapper for Conan,
and Conan as a wrapper for CMake. The following files are present in the root
directory of every Cloe package (e.g. in ``plugins/basic/``):

``Makefile``
    Contains a set of targets that can be run in this directory. For most
    packages, the ``Makefile.package`` from the top-level directory is included
    to provide Make targets for the most common Conan commands. Refer to the
    output of ``make help`` in the respective package directory for a list of
    the available targets (here: ``make -C ./models help``):

    .. comment:
       The path below is relative to the project docs folder, not this file.

    .. runcmd:: bash -c "make -C ../models help | sed -e 's/\x1b\[[0-9;]*m//g'"
       :replace: "PACKAGE_DIR:.*\\//PACKAGE_DIR:     <SOME_PATH>\\/"

``conanfile.py``
    Conan package recipe. To gain a thorough understanding of the Conan methods
    and tools used therein, the `Conan CMake integration`_ and
    `Conan recipe reference`_ documentation is probably a good starting point.

``CMakeLists.txt``
    Contains cross-platform instructions for how to build, test, and install the
    package. The configuration is done via Conan. You may come across CMake
    Cloe-specific CMake functions such as ``cloe_add_plugin``, which are defined
    in ``runtime/cmake/CloePluginSetup.cmake`` in the top-level directory.
    Conan-specific declarations are contained in ``conanbuildinfo.cmake``, which
    will be generated after ``conan install`` is invoked.


In-Source Builds
----------------

Suppose you are making changes to the basic controller plugin and you would like
to build your modified package in your local working directory. To do this,
you may build `Conan packages in editable mode`_. Try it out::

 cd plugins/basic/
 make editable

Verify that the cloe-plugin-basic package is currently in editable mode::

 make status

.. note::
   Positive-Example::

      editable : cloe-plugin-basic/0.18.0-rc5-3-g53c80db@cloe/develop

   Negative-Example::

      ok       : cloe-plugin-basic/0.18.0-rc5-3-g53c80db@cloe/develop

Now, you can build the package binaries in your local working directory::

 make clean all

The next time when Conan needs the package ``cloe-plugin-basic`` in
this version, it will resolve the include and library directories
to this local build. It is important to understand that you as a
developer are now responsible for ABI compatibility!!

.. note::
   Conan can build packages with any constellation of dependencies that
   you may require. This means that it is necessary to build an individual
   package in a way that is compatible with the final composition.

   For example, it may be that the entire set of packages as defined by
   the ``cloe-meta`` package require ``boost/1.65.1``. When building
   the ``basic`` plugin as in this example, it has no way of knowing
   that this version of Boost will be used when building ``cloe-engine``.
   Therefore Conan will use the latest version of the ``boost`` package
   it can find, such as ``boost/1.78.0``.

   In normal non-editable builds, Conan tracks these potential
   incompatibilities and prevents you from incorrect combinations.
   In editable mode however, you are responsible. Combining code
   linked to two different versions of Boost is undefined behavior
   and will lead to segfaults or worse!

   The solution to this dilemma is to let Conan know when making
   a local build to use the final composition configuration for
   resolving dependency configurations. This can be done by
   generating a lockfile first of the final composition, and
   using this lockfile when building a part locally.

   This is common enough that there is a simple mechanism baked
   into the Makefiles to use a Conan recipe for automatically
   generating and using a lockfile::

     make clean all LOCKFILE_SOURCE=${CLOE_ROOT}/conanfile-meta.py

   where ``${CLOE_ROOT}`` is the path to the repository root; for
   the ``basic`` plugin, this is ``../..``.

Since the package is in editable mode, the binaries will be created in the
``./build/`` directory in your package sub-directory.

When you finalize work on your package, remember to remove the editable mode::

 make uneditable

Verify the package status::

 make status

If you execute the latter command from the top-level directory, you will see the
status of all Cloe packages.

Practical Example
"""""""""""""""""

Let's apply the above to a very practical example involving ``cloe-engine``.
Assume the following situation: I checkout a develop branch, such as
``develop``, with the intention of modifying the ``cloe-engine`` package.

First, because I am going to make changes, I disable the use of ``git describe``
for versioning by explicitely setting a version::

    echo "0.99.0-develop" > VERSION

Then I make sure the entire project is exported::

    make export-all

.. note::
   This is roughtly equivalent to::

     ( cd fable && conan export conanfile.py fable/0.99.0-develop@cloe/develop)
     ( cd runtime && conan export conanfile.py cloe-runtime/0.99.0-develop@cloe/develop)
     ...

If there are any changes in other packages, I want to pick up those as well.
I let Conan know that I want to use ``cloe-engine`` in editable mode::

    cd engine
    make editable

.. note::
   This is equivalent to::

     cd engine
     conan editable add conanfile cloe-engine/0.99.0-develop@cloe/develop

Now, I need to choose a configuration that I want to use for testing the
entire set of packages. I can use ``cloe-meta`` or I can use a configuration
in the ``tests/`` directory, such as ``tests/conanfile_split.py``.
I use this when building ``cloe-engine`` as the source for creating a lockfile::

    cd engine
    make clean all LOCKFILE_SOURCE=../tests/conanfile_split.py

This will automatically build any missing dependencies that are necessary for
building ``cloe-engine``, after which it will build ``cloe-engine`` locally.

Before running any tests, I may need to make sure any additional dependencies
not required by ``cloe-engine`` but required for the test execution are
built::

    cloe-launch prepare tests/conanfile_split.py

.. note::
   This is *approximately* equivalent to::

     conan install ../tests/conanfile_split.py --build=outdated --build=cascade

Once this is complete, we can launch into a correctly configured shell,
with ``PATH`` and ``LD_LIBRARY_PATH`` set so that the shell can find
``cloe-engine`` and it can find required libraries and plugins.

.. code-block::

   cloe-launch shell -c tests/conanfile_split.py

And at this point we are done and can run tests, make modifications
to the editable ``cloe-engine`` package, rebuild, and run tests again::

    bats tests
    $EDITOR engine/src/main.cpp
    make -C engine all
    bats tests


Superbuild
----------

The practical example of in-source builds above was one of the easiest
configurations we could choose. It becomes more arduous when we want to
edit packages that other packages in turn depend on, because then we need
to compile multiple packages by hand in the correct order.

To side-step all of this, we have the ``cloe`` package, which is a
super-build of all other packages.

You can build in the Conan cache with::

    make package

You can build it locally with::

    make all

You can then launch a virtual environment with ``cloe-launch``::

    cloe-launch shell tests/conanfile_all.py

.. _Conan local cache: https://docs.conan.io/en/latest/mastering/custom_cache.html
.. _Conan CMake integration: https://docs.conan.io/en/latest/integrations/build_system/cmake.html
.. _Conan recipe reference: https://docs.conan.io/en/latest/reference/conanfile.html
.. _Conan packages in editable mode: https://docs.conan.io/en/latest/developing_packages/editable_packages.html
