Makefile.package
================

The ``Makefile.package`` file is a Makefile that provides a handfull of targets
(recipes) for dealing with Conan packages in this repository. The provided
targets are documented in the file and in the ``help`` target, so you know
which targets we think you should know about and also what they do.
This is incidentally also good way to see which Conan commands we think
you should know about when going about interacting with Conan packages.

 .. note::
    The ``Makefile.package`` file is designed in such a way that you can
    easily copy it to another repository.

Generally, you should not need to specify the Makefile itself when calling
make, so packages that make use of this Makefile should create a
``Makefile`` file and then include ``Makefile.package`` like so:

 .. code:: make

    include ../Makefile.package

Then in the directory with the Makefile, you can simply run ``make help`` and
you will see the help:

 .. runcmd:: make --no-print-directory -C ../fable help

 .. todo::
    This file is a work-in-progress and will be extended as we find time.

status
------

The ``status`` target prints information about Conan's view of the current
package. This is specific to the package name, version, user, channel, and
profile, such as settings and package options, as well as resolved
dependencies. For this reason, it is recommended to use the ``LOCKFILE_SOURCE``
option where reasonable, since you otherwise may receive incorrect answers.

 .. note::
    This is a somewhat more involved target, since it by-passes Conan to provide
    some of the more valuable information. Normally, we try not to do this much
    "work" in Makefiles, but this target is used often enough to warrant it.

The following descriptive words are used, prepended to the name of the package
in question:

ok
   Package is built and up-to-date.

outdated
   Package is not available or source is newer.

editable
   Package is editable.

editable-other-name
   Package path is editable, but by another name.
   This can happen when you set editable and then change the
   version of the package by either setting VERSION or by
   checking out another state of the code.

editable-elsewhere
   Package is editable, but from a different location.
   This can happen when you have multiple clones of the
   repository at different locations.
