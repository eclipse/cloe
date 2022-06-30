Running the Cloe Engine
=======================

.. highlight:: console

Now that we have Cloe :doc:`installed <../install>` and understand how Cloe
separates its functionality into Conan packages, let's try to run the engine
(``cloe-engine``), which acts as the middleware in the simulation framework, as
well as supplying a web server and some other nifty features.

If you search through the Conan cache, you can find out which ``cloe-engine``
packages you have built::

    $ conan search cloe-engine
    Existing package recipes:

    cloe-engine/0.18.0-rc3@cloe/develop
    cloe-engine/0.18.0-rc4@cloe/develop
    cloe-engine/0.18.0@cloe/develop

But how do you run one of these? If you look into the package directory of
the Conan package, you will find a ``bin`` directory, which contains the
``cloe-engine`` executable, but if you try to run it, you will get an error::

    $ cd ~/.conan/data/cloe-engine/0.18.0/cloe/develop
    $ ls
    build/  export/  export_source/  locks/  metadata.json  metadata.json.lock  package/  source/
    $ tree package
    package
    └── e228d2ea7272de48dde779ca1121f71dc15701ef
        ├── bin
        │   └── cloe-engine
        ├── conaninfo.txt
        └── conanmanifest.txt
    $ cd package/e228d2ea7272de48dde779ca1121f71dc15701ef
    $ bin/cloe-engine --help
    bin/cloe-engine: error while loading shared libraries: libcloe-runtime.so.0.18.0: cannot open shared object file: No such file or directory

Without the library contained in ``cloe-runtime``, we can not start the engine.
One approach to use a tool from the Conan cache is to create a `virtual
environment`_, which is conceptually very similar to what the Python
``virtualenv`` tool does::

    $ mkdir /tmp/cloe-engine-virtualenv
    $ cd /tmp/cloe-engine-virtualenv
    $ conan install cloe-engine/0.18.0@cloe/develop -g virtualrunenv
    Configuration:
    [settings]
    arch=x86_64
    arch_build=x86_64
    build_type=RelWithDebInfo
    compiler=gcc
    compiler.libcxx=libstdc++11
    compiler.version=8
    os=Linux
    os_build=Linux
    [options]
    [build_requires]
    [env]

    [...]

    cloe-engine/0.18.0@cloe/develop: Appending PATH environment variable: /home/captain/.conan/data/cloe-engine/0.18.0/cloe/develop/package/e228d2ea7272de48dde779ca1121f71dc15701ef/bin
    Generator virtualrunenv created activate_run.ps1
    Generator virtualrunenv created deactivate_run.ps1
    Generator virtualrunenv created environment_run.ps1.env
    Generator virtualrunenv created activate_run.sh
    Generator virtualrunenv created deactivate_run.sh
    Generator virtualrunenv created environment_run.sh.env
    Generator txt created conanbuildinfo.txt

Conan outputs a lot of information, so it's been snipped in the middle.
The important thing to note here is that the virtualrunenv Conan generator
created a file ``activate_run.sh`` which sets the environment variables
required to run ``cloe-engine``.

With ``source ./activate_run.sh`` you can activate the virtual run environment.
Don't be afraid to look at the contents of these scripts; they are pretty
simple. Now that the environment is set up, we should be able to run the
engine::

    $ source ./activate_run.sh
    $ cloe-engine version
    Cloe 0.18.0

    Engine Version:  0.18.0
    Build Date:      2021-06-25
    Stack:           4
    Plugin Manifest: 1

Excellent. Note that ``cloe-engine`` has been added to our PATH, so we can run
it from any directory.

.. note::
   With the `virtualrunenv` approach you can quickly switch between different
   versions of Cloe in your cache or use them at the same time from different
   shells.

View Help and Version
---------------------

If we ever have problems with Cloe, or are unsure how we can use the engine,
it can be helpful to view the help. To do this, pass the ``-h``, ``--help``, or
``--help-all`` flag::

    $ cloe-engine --help-all
    Cloe 0.18.0
    Usage: cloe-engine [OPTIONS] SUBCOMMAND

    Options:
      -h,--help                   Print this help message and exit
      -H,--help-all               Print all help messages and exit
      -l,--level TEXT             Default logging level
      -p,--plugin-path TEXT ...   Scan additional directory for plugins
      -i,--ignore TEXT ...        Ignore sections by JSON pointer syntax
      --no-builtin-plugins        Disable built-in plugins
      --no-system-plugins         Disable automatic loading of system plugins
      --no-system-confs           Disable automatic sourcing of system configurations
      --no-hooks                  Disable execution of hooks
      --no-interpolate{false}     Interpolate variables of the form ${XYZ} in stack files
      --interpolate-undefined     Interpolate undefined variables with empty strings

    Subcommands:
      version                     Show program version information.
      usage                       Show schema or plugin usage information.
      dump                        Dump configuration of (merged) stack files.
      check                       Validate stack file configurations.
      run                         Run a simulation with (merged) stack files.

It's also highly highly recommended to read the output of ``cloe-engine usage``,
as this will give you a good overview of what you can use each of the
subcommands for.

Generally the command used here ``cloe-engine --help`` should be used first when
you have a problem executing the program to double-check whether you are doing
the right thing or not. Cloe will tell you if you do something wrong though::

   $ cloe-engine
   A subcommand is required.
   Run with --help or --help-all for more information.

When asking for help, sending the developers of Cloe the specific version you
are using is extremely helpful. This can be achieved by running::

    $ cloe-engine version
    Cloe 0.18.0

    Engine Version:  0.18.0
    Build Date:      2021-06-25
    Stack:           4
    Plugin Manifest: 1

However, generally the initial version line in a normal Cloe session is
sufficiently precise.

In the next session, we'll introduce the ``cloe-launch`` tool.

.. _virtual environment: https://docs.conan.io/en/latest/mastering/virtualenv.html#virtual-environment-generator

----

.. rubric:: Suggested Exercises

#. | Display the list of available Conan packages in your cache.
   | (Hint: See what the ``search`` subcommand can do for you.)

#. | Inspect the contents of a Conan package.

#. | Create a Conan virtual run environment for ``cloe-engine``.

#. | Check the output of ``cloe-engine version``.

#. | Read the output of ``cloe-engine usage``.
