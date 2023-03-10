Frequently Asked Questions
==========================

What 3rd-party libraries do you use?
""""""""""""""""""""""""""""""""""""

We use several open source libraries as part of Cloe, some of which we ship with
the Cloe source code. The list of Cloe dependencies and these included libraries
and their licenses can be found in the ``LICENSE-3RD-PARTY.txt`` file.

We provide simulator bindings to two commercial simulators. These bindings use
development files from these simulators, in part through simple includes,
in part through patch-files, as is technologically necessary.

- Vires VTD

Development tools we currently use include the following, but
is not limited to:

- GCC
- Git
- Python
- Clang
- CMake
- Make
- Bash, Zsh
- Docker
- Doxygen


How do I avoid constant version changes like "0.18.0-38-g46e9b0b"?
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

Generally, producing an immutable package is a good thing. Each commit that
can be checked out should produce a differently versioned package, since the
contents are also different. During development, this can produce a plethora
of packages and leads to forced recompilation every time a commit is made or
the git hash is changed.

For example, running ``make smoketest-deps smoketest`` in the background and
then committing work while it is running will lead to smoketests failing
because Conan can't find the new package version.

The solution is to set the version explicitly, which can be done by writing
a string to the ``VERSION`` file in the repository root. This version should
have a suffix that makes clear that it is a volatile package, such as
``-nightly``. For example, in the repository root:

    echo "0.19.0-nightly" > VERSION

The ``VERSION`` file is listed in the ``.gitignore`` and is not committed.
Just remember when you are using it, that every package you compile from that
repository will have that version, even if you check out different states.


How do I get rid of the "-dirty" suffix on my versions?
"""""""""""""""""""""""""""""""""""""""""""""""""""""""

This indicates you have uncommited changes in the repository. Commit your
changes or set the version explicitly in the ``VERSION`` file in your
repository root.

See the previous question.
