Building a Cloe Package
=======================

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

    .. runcmd:: make -C ../models help
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
 Positive-Example:

 .. code-block::

  editable : cloe-plugin-basic/0.18.0-rc5-3-g53c80db@cloe/develop

 Negative-Example:

 .. code-block::

  ok       : cloe-plugin-basic/0.18.0-rc5-3-g53c80db@cloe/develop

Now, you can build the package binaries in your local working directory::

 make clean all

Since the package is in editable mode, the binaries will be created in the
``./build/`` directory in your package sub-directory.

When you finalize work on your package, remember to remove the editable mode::

 make uneditable

Verify the package status::

 make status

If you execute the latter command from the top-level directory, you will see the
status of all Cloe packages.


Testing Cloe
------------

With the help of CMake and the Google Test framework, unit tests can be run
with::

    make test

Available unit tests are automatically executed after each package was built, if
the Conan recipe option ``test`` is set to ``True`` (default: ``True``).


Distributing Cloe Packages
--------------------------

It is recommended to distribute Cloe binary packages via your Conan remote
repository. To understand the workflow and upload packages to a local Conan
server for testing, refer to the provided dockerized Conan server example
(s. ``tools/conan-server/README.md``) and the `Conan package upload`_
documentation.

The Cloe package release process has been automated for multiple Linux
distributions. Please make sure you understand the instructions in
``dist/docker/README.md``, particularly with respect to the provided
``dist/docker/setup.sh.example`` file.

You may start the release process from the top-level directory like so::

  make docker-release

This will build and test all Cloe packages in multiple Linux environments and
upload the binaries to the Conan remote repository specified in
``dist/docker/setup.sh``.


.. _Cloe repository: https://github.com/eclipse/cloe
.. _Conan local cache: https://docs.conan.io/en/latest/mastering/custom_cache.html
.. _Conan CMake integration: https://docs.conan.io/en/latest/integrations/build_system/cmake.html
.. _Conan recipe reference: https://docs.conan.io/en/latest/reference/conanfile.html
.. _Conan packages in editable mode: https://docs.conan.io/en/latest/developing_packages/editable_packages.html
.. _Conan package upload: https://docs.conan.io/en/latest/uploading_packages.html
