Integrating a Controller
========================

When it comes to writing your own Cloe plugins you'll need to build against a
version of Cloe. Since you know now how to create your Cloe package with Conan
it's convenient to use that package.

You'll simply write a `conanfile.txt <https://docs.conan.io/en/latest/reference/conanfile_txt.html>`__
in your plugin's root directory containing the following:

.. code-block:: txt

   [requires]
   cloe/0.20.0@cloe/develop

   [generators]
   CMakeDeps
   CMakeToolchain

   [layout]
   cmake_layout

It states the version of the Cloe package to use. All dependencies of Cloe will
be inherited and should not be re-stated unless you explicitly want to use a
particular version of a dependency here.

Now run ``conan install . --build=missing --install-folder=build``.
This ensures that Cloe and its dependencies are cached and will create a
toolchain file called ``conan_toolchain.cmake`` in the ``build/generators``
folder. Include this file from the command line when calling CMake
and then ``find_package`` your requirements like:

.. code-block::

   find_package(cloe-runtime REQUIRED)
   find_package(cloe-models REQUIRED)

See the example projects in ``fable/examples`` for some examples of how Conan
is used. If any of this is new to you and you find yourself working with Conan,
then it is also highly recommended to take a Conan course from JFrog (free
of charge) to get familiar with Conan.

Now you can refer to cloe as a dependency library in your
``target_link_libararies`` statement like ``cloe::runtime``.

.. todo:: Write a section on how to specify a specific version of Cloe.
   There are multiple use-cases here:
   - Following HEAD
   - Using an specific version of Cloe
   - Using a specific branch/tag of Cloe
   - Using a fork of Cloe
