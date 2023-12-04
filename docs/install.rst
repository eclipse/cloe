Installation
============

For building, deploying, and running the runtime and engine we use `Conan`_,
a modern C++ package manager. We currently do not have any published Conan
packages that can be downloaded directly. Building them yourself is pretty
straightforward.

Currently, we only officially support Linux or `WSL`_.

The minimum required C++ standard is 17, which is provided by a minimum
GCC version of 8. This is available from Ubuntu 18.04.

Install Dependencies
--------------------
We provide automatic dependency installation for `Ubuntu`_ from 18.04 to 22.04
via the ``Makefile.setup`` Makefile. You should inspect it before running the
targets, as these will modify your system. Other distributions may work, if the
packages are available.

::

    git clone https://github.com/eclipse/cloe.git
    cd cloe
    sudo make install-system-deps
    make install-python-deps

Configure Conan
---------------
You may need to setup your Conan profile before continuing on to the next
point. The recommended procedure is to use the make target::

    make setup-conan

In a pinch though, the following steps should suffice:

1. `Install Conan <https://docs.conan.io/en/latest/installation.html>`__,
   if you haven't done so already; for example with `pip`_ or `pipx`_::

      pipx install "conan<2.0.0"

   .. note::
      We expect you to keep your Conan installation up-to-date. Since Conan is
      actively developed, bugfixes and improvements are frequently published.

2. Define a Conan profile, which defines the machine configuration::

       conan profile new --detect default
       conan profile update settings.compiler.libcxx=libstdc++11 default

   If everything works out, your Conan profile should look something like
   this::

       $ conan profile show default
       Configuration for profile default:
       [settings]
         os               = Linux
         os_build         = Linux
         arch             = x86_64
         arch_build       = x86_64
         compiler         = gcc
         compiler.version = 9
         compiler.libcxx  = libstdc++11
         build_type       = Release

See the `Conan documentation`_ for more information on how to do this.

Build Cloe Packages
-------------------
To build all packages, you should run the following::

    make export-all
    make -C meta package

This will export all Conan recipes from this repository and create the cloe
package. Conan will download and build all necessary dependencies. Should
any errors occur during the build, you may have to force Conan to build
all packages instead of re-using packages it finds::

    make -C meta package CONAN_OPTIONS="--build"

.. note::
   Depending on your Conan profile, building the Cloe packages can involve
   building other dependency packages which are not available as pre-built
   binaries in your Conan cache or your default remote. Downloading the source
   code and building will take some time but will be handled automatically by
   Conan and will take place in the Conan cache directory.

If you like, you can inspect what a Conan Cloe package looks like by browsing
the Conan cache directory under ``~/.conan/data/cloe``.

Run ``make help`` to get an overview of the available targets we anticipate you
may want to use. For more details on how this is done, have a look at the
Makefiles in the repository root.

Run System Tests
----------------
To check that everything is working as it should, we recommend you run the
included test suite once before commencing with anything else::

    make export-all smoketest-deps smoketest

.. _Conan: https://conan.io
.. _Conan documentation: https://docs.conan.io/en/latest/
.. _performance issues: https://github.com/conan-io/conan-center-index/issues/950
.. _WSL: https://docs.microsoft.com/en-us/windows/wsl/about
.. _Ubuntu: https://ubuntu.com
.. _pipx: https://pypa.github.io/pipx/
.. _pip: https://pypi.org/project/pip/
