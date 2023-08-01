Testing Cloe
============

 .. note::
    Please ensure you :doc:`understand<../usage/understanding-cloe-packages>`
    how Cloe packages work before reading this section.

Cloe packages need to be tested in isolation as well as together with other
packages. To this end, packages may include unit tests, example executables,
and integration tests.

Unit Tests
----------

With the help of CMake and the Google Test framework, unit tests (written in
C++) can be run within each package directory with::

    make test

Note that this requires a local build (``make all``) to work. Available unit
tests are automatically executed after each package was built, if the Conan
recipe option ``test`` is set to ``True`` (default: ``True``). This means,
a successful build usually includes successful test execution, such as when we
run::

    make package

or::

    make all

It's therefore usually not necessary to explicitly run the unit tests, unless
there is a failing test.

To understand what's going on, it's recommended to check out the targets as
defined in the ``Makefile.package`` file in the repository root.

When tests are executed automatically, CMake collates available tests and runs
them with its own command line output. If a test fails, all you see is that
a test failed, but you won't see *why* it failed, nor will you directly be able
to debug the test. In order to that, you need to find the test executable that
CMake executed and run that yourself.

For example, if you compiled with ``make all``, then you can check the
``build/bin/`` directory for a test executable. This you can run directly to
see a more detailed error report. You can also run the executable with GDB
if you want to debug the test step-for-step::

    gdb build/bin/test-engine

 .. note::
    Depending on your build settings, the unit test may dynamically link to
    libraries provided by other Conan packages. If this is the case, you can
    use the cloe-launch tool to create a shell in which to run the unit test.

When using GDB, it may be helpful to tell GDB to ``catch throw``
(see `GDB Catchpoints<https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_30.html>`__).

Integration Tests
-----------------

Running an integration test involves combining multiple Conan packages into
a cohesive unit and running a Cloe simulation with a stack file. To streamline
this we generally use ``cloe-launch`` to wrap the temporary installation of
a virtual runtime environment, activating this environment, and running
commands within the environment.

Integration tests are located throughout the repository in ``tests/``
directories. For example, the minimator plugin has this in the ``tests``
directory, as of this writing::

    plugins/minimator/tests
    ├── config_minimator_infinite.json
    ├── config_minimator_multi_agent_infinite.json
    ├── config_minimator_smoketest.json
    ├── conanfile_default.py
    ├── test_minimator.bats
    ├── test_minimator_multi_agent_smoketest.json
    └── test_minimator_smoketest.json

Each directory has different files, following the patterns below:

``conanfile_*.py``
    Each of these files are cloe-launch profiles (i.e., Conanfiles) which should
    be used to test all of the stack files in the same directory.

    Note that this may involve re-compilation of one or more parts of Cloe.
    For example, one profile may use a configuration that has the server in
    cloe-engine disabled, another may force a specifc boost version, and yet
    another may use a different compiler or toolchain.

    These can actually be used by Conan directly::

        # mkdir virtualenv && cd virtualenv
        # conan install ../conanfile_default.py
        # source cloe_launch_env.sh
        # source activate.sh
        # source activate_run.sh

    Using cloe-launch automates that process and adds some flags for a one-stop
    shop::

         # cloe-launch shell conanfile_default.py

``config_*.json``
    Each of these files provides some base configuration that can be re-used by
    other tests. They are generally complete stack files that can be checked or
    run with ``cloe-engine`` inside a virtual run environment.

``controller_*.json``
    The controller configuration is used in both NOP as-well-as VTD simulator
    binding stack files. To reduce duplication, the controller specific
    configuration is placed in its own partial configuration file. This file
    can be checked inside a cloe shell, but it might fail::

        # cloe-engine check tests/controller_basic.json
        tests/controller_basic.json: cannot find a vehicle with the name 'default': no entity with that name has been defined

``test_*.json``
    Each of these files is used for a specific test in a BATS file. In the
    simplest case, it only includes another stack file.

``test_*.bats``
    Test executions are written in Bash and these tests are written with the
    BATS framework.

``setup_*.bash``
    These files contain definitions that are used in all specific BATS scripts.
    It should not be used directly, as it is loaded in each bats script.


Testing stack files with Cloe is automated with the
`Bash Automated Testing System <https://github.com/bats-core/bats-core>`__ (BATS).
This requires the *bats* tool to be installed::

    # sudo apt-get install bats

Once installed, BATS can be used with individual files or with the entire
directory::

    # cloe-launch shell tests/conanfile_default.py -- -c "bats tests"
    ✓ Expect check success    : test_minimator_smoketest.json                  : c7a427e7-eb2b-4ae7-85ec-a35b7540d4aa
    ✓ Expect run success      : test_minimator_smoketest.json                  : 7c67ceb9-3d1d-47e4-9342-0b39099c59d6
    ✓ Expect check/run success: test_minimator_smoketest.json [ts=5ms]         : 57254185-5480-4859-b2a5-6c3a211a22e0
    ✓ Expect check/run success: test_minimator_smoketest.json [ts=60ms]        : a0d4982f-8c02-4759-bc88-cc30a1ccbbf0
    ✓ Expect check/run success: test_minimator_multi_agent_smoketest.json      : 90e440ec-e8bc-40bb-8d2a-de224ee872bb

    5 tests, 0 failures

You can also use the ``make smoketest`` target, which is defined in
``Makefile.package`` and should be available in each package directory.

Test UUIDs
""""""""""

In order to make tracking quality measures possible, we assign tests UUIDs.
These can be generated with the `uuidgen` program, for example. When creating
a large amount of UUIDs, it can be useful to augment one's editor with the
capability of inserting one quickly. For example, to map it to Ctrl+Y in Vim:

    :imap <c-y> <esc>:read !uuidgen<cr>kJi

Search the project for UUIDs with:

    # make grep-uuids

Test Automation
"""""""""""""""

In order to run all integration tests for a project, we need to consider that
we will have to build multiple configurations and maybe even run the same set of
tests with different configurations. To that end, ``Makefile.package`` provides
two targets:

``smoketest-deps``
    Use ``cloe-launch prepare`` to pre-compile the different profiles found
    in the ``tests/`` directory and used for the integration tests.

    This requires that you have exported the sources to Conan first, with
    the ``export`` target for an individual package, or ``export-select``
    for the entire project.

    In general, you will need to export or package the entire project at least
    once.

``smoketest``
    Run all BATS tests for each profile found in the ``tests/`` directory.

This can be done on a package by package basis, or you can do this project
wide.
