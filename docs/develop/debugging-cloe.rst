Debugging Cloe
==============

Debugging is part of development, and unless you're the perfect developer, you
will probably want to do this from time to time. This section contains a few
tips for debugging Cloe components and Cloe as a system.

 .. note::
    It is recommended you understand how :doc:`cloe-launch<../usage/using-cloe-launch>`
    works before reading this section. The document on :doc:`testing
    cloe<../develop/testing-cloe>` is also useful in the context.

Conan Profiles
--------------

You can manage multiple Conan profiles in the ``~/.conan/profiles/`` directory.
The default profile that is used can be configured in ``~/.conan/conan.conf``.

A profile looks like this::

    [settings]
    os=Linux
    os_build=Linux
    arch=x86_64
    arch_build=x86_64
    compiler=gcc
    compiler.version=10
    compiler.libcxx=libstdc++11
    build_type=RelWithDebInfo
    [options]
    [build_requires]
    [env]
    CC=/usr/bin/gcc-10
    CXX=/usr/bin/g++-10

You can set the ``build_type`` setting to something that CMake understands,
such as ``Release``, ``Debug``, or ``RelWithDebInfo``. It is generally
recommended to use the ``RelWithDebInfo``, as this mode strikes a balance
between optimization and debugging information.

If you set this to ``Debug``, you'll find that Conan suddenly starts to
rebuild all dependencies the next time you build your project. You can
avoid this by using a little trick::

    build_type=RelWithDebInfo
    *@cloe/develop:build_type=Debug

Logging
-------

Start the Cloe runtime with the ``-l <logging-level>`` parameter to specify
the logging level that should be used. For example::

    # cloe-engine -l debug run tests/config_vtd_smoketest

The following log levels are understood: ``trace``, ``debug``, ``info``,
``warn``, ``error``, and ``critical``.

Using GDB
---------

First of all, make sure you have an up-to-date version of GDB. This is
especially relevant when using a more recent version of GCC than the Ubuntu
version you are using is shipped with::

    # sudo apt-get remove gdb
    # sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    # sudo apt-get update
    # sudo apt-get install --install-suggests gcc-9 g++-9 gdb

Then for compiling the project you should compile with the latest version of
GCC so that it plays well with GDB. You need to set this in your Conan profile,
as seen in the previous section on Conan profiles.

Using GDB with cloe-launch
""""""""""""""""""""""""""

When directly starting a simulation with ``cloe-launch exec``, you can provide
the ``--debug`` argument to start GDB with the arguments automatically
set to the ones passed through cloe-launch::

    # cloe-launch exec -d tests/conanfile_default.py -- run tests/config_nop_smoketest.json

You will be dropped in a GDB shell, where you can set your breakpoints as
usual, and then to start debugging the program, just type ``run``.

Scripting with GDB
""""""""""""""""""

GDB is great, but it can be a bit arcane at times. One solution that often
works is to use a competent IDE like Visual Studio Code, but this doesn't
always work, for example when you are debugging within a container.
GDB can be automated however, and this brings it to a complete new level: now
your debugging sessions can be fully programmed.

For example, given the following contents in a file ``debug.gdb``:

 .. code-block:: text

    # vim: set ft=gdb:

    # Skip asking for confirmation.
    set breakpoint pending on

    # Ensure that we are actually passing the configurations into the factory.
    break vtd::VtdFactory::apply_config
    commands 1
       printf "VtdFactory\n"
       printf "-- c.root: %s\n", c.root_.c_str()
       continue
    end

    # Check what the VtdConfiguration is when we initialize a Vtd.
    # We expect that: setup = "Cloe.noGUInoIG"
    break vtd::VtdBinding::VtdBinding
    commands 2
       p config
    end

    # Run the program
    run -l debug run optional/vtd/tests/test_vtd_smoketest.json

We can then run this script inside a Cloe shell
(``cloe-launch shell optional/vtd/tests/default_profile.py``) with::

    # gdb --command=debug.gdb cloe-engine

And presto! We're back in business!
