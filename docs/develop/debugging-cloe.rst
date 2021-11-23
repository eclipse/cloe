Debugging Cloe
==============

Debugging is part of development, and unless you're the perfect developer, you
will probably want to do this from time to time. This section contains a few
tips for debugging Cloe components and Cloe as a system.

Logging
-------

Start the Cloe runtime with the ``-l <logging-level>`` parameter to specify
the logging level that should be used. For example::

   build/cloe-engine -l debug run tests/config_vtd_smoketest

GDB Wizardry
------------

First of all, make sure you have an up-to-date version of GDB. This is
especially relevant when using a more recent version of GCC than the Ubuntu
version you are using is shipped with::

   sudo apt-get remove gdb
   sudo add-apt-repository ppa:ubuntu-toolchain-r/test
   sudo apt-get update
   sudo apt-get install --install-suggests gcc-9 g++-9 gdb

Then for compiling the project you should compile with the latest version of
GCC so that it plays well with GDB::

   export CC=/usr/bin/gcc-9 CXX=/usr/bin/g++-9
   ./configure

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
   run -l debug run tests/test_vtd_smoketest.json

We can then run this script inside a Cloe shell
(``cloe-launch shell -P conantest.py -o:o with_vtd=True``) with::

   gdb --command=debug.gdb cloe-engine

And presto! We're back in business!
