Integrating a Controller
========================

When it comes to writing your own Cloe plugins you'll need to build against a
version of Cloe. Since you know now how to create your Cloe package with Conan
it's convenient to use that package.

You'll simply write a `conanfile.txt <https://docs.conan.io/en/latest/reference/conanfile_txt.html>`__
in your plugin's root directory containing the following:

.. code-block:: txt

   [requires]
   cloe/0.17.0

It states the version of the Cloe package to use. All dependencies of Cloe will
be inherited and should not be re-stated unless you explicitly want to use a
particular version of a dependency here.

Now run ``conan install . --install-folder build``. This ensures that Cloe and
its dependencies are cached and will create a file called
``conanbuildinfo.cmake`` in the ``build`` folder. Include this file in your
CMakeLists.txt (we assume you use CMake for your plugin project) like:

.. code-block::

   include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
   conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

Now you can refer to cloe as a dependency library in your
``target_link_libararies`` statement like ``CONAN_PKG::cloe``.


.. todo:: Write a section on how to specify a specific version of Cloe.
   There are multiple use-cases here:
   - Following HEAD
   - Using an specific version of Cloe
   - Using a specific branch/tag of Cloe
   - Using a fork of Cloe
