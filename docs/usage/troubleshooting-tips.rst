Troubleshooting Tips
====================

This section contains a collection of useful troubleshooting tips that you can
use when things don't work as expected.

Increase the Launcher Verbosity
-------------------------------
You can increase the verbosity of the ``cloe-launch`` tool by adding one or
more ``-v`` flags. This will show you the exact environment that is used.

Increase the Engine Verbosity
-----------------------------
You can force ``cloe-engine`` logging to use the debug or trace level with the
``--log-level debug`` argument.

Preserve the User Environment
-----------------------------
By default, the launcher tries to minimize the environment. If this doesn't
work for you, you can pass the ``-E`` flag to the launcher to force it to
preserve the environment.

Add Environment Variables To Conan Profile
------------------------------------------
If you are using Pyenv, make sure to add your shims to your Conan profile::

    ...
    [env]
    PATH=[/home/captain/.pyenv/shims]

Also refer to the :doc:`../install` section for more details on how to configure
Conan.
