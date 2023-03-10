Understanding Cloe Packages
===========================

.. highlight:: console

This section assumes that you have cloned and compiled Cloe.
If not, see the previous section, :doc:`../install`.

Cloe is not a single tool. It is a set of components written in different
languages employing different tools that work together:

- Cloe runtime & engine (C++)
- Cloe command line interface (Python)
- Cloe web user interface (Javascript/React)

The basis of a simulation is the runtime and engine. As described in
:doc:`../overview`, plugins supplied by the Cloe team and users of Cloe are
integrated at runtime into a simulation. In order to give the users and
developers the greatest flexibility in choosing what is part of a simulation,
these plugins are built as separate `Conan`_ packages. This allows you to
include and exclude them at will, as well as specify alternate versions.

.. image:: package-hierarchy.png
   :alt: Cloe package hierarchy

Each of the boxes in the image above represents a single Conan package. The
set of packages is not exhaustive, as implied by the boxes with ellipsis.
The arrows define dependencies between packages. For example, ``cloe-engine``
depends on ``cloe-runtime``. Most plugins depend on ``cloe-models``, which
in turn depends on ``cloe-runtime``. Some packages, like ``oatpp`` are
not developed by us, but packaged into Conan packages by us. Other dependencies
(not shown) such as Boost, are retrieved from the Conan Center, which contains
a large collection of Open Source libraries and tools.

When you build Cloe with Conan, you are not building a single package; rather,
the entire set of packages are built or retrieved that are required. We also
provide a ``cloe`` meta-package, which has no assets itself, just referring to
the full set of Cloe packages that are built. Conan can show us the dependency
graph::

    $ conan info -n cloe/0.18.0-rc4@cloe/develop
    [...]
    boost/1.69.0
        ID: 723638df3c23ba2e9b285a2bd692e0abeea9b6e3
    bzip2/1.0.8
        ID: 958b011845d8998ef3cc45a6238d89fc6cb91e6b
    cli11/1.9.1
        ID: 5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9
    cloe/0.18.0-rc4@cloe/develop
        ID: 7f32dc2e800f89776fadf3e808f1cc2a161b7801
    cloe-engine/0.18.0-rc4@cloe/develop
        ID: 285f8581e0e09ea56cf4899a59a086733697e94e
    cloe-models/0.18.0-rc4@cloe/develop
        ID: 96a472ee5c9a853c268baa6f178e9d677231ff5d
    cloe-oak/0.18.0-rc4@cloe/develop
        ID: 4d306d383caa7c5996463f457869ee1dce9e9b4b
    cloe-plugin-basic/0.18.0-rc4@cloe/develop
        ID: c3e77167465b16a65fa1a89c30aefd819df3bf99
    [...]
    cloe-plugin-virtue/0.18.0-rc4@cloe/develop
        ID: 80df036a0370dac33e0516aff16b753ba74e8428
    cloe-runtime/0.18.0-rc4@cloe/develop
        ID: 10d8b960d2a22d3e4d50a5e95361462cb55854e9
    cpp-netlib/0.13.0@cloe/stable
        ID: 445bedf3c6f1338d4d80680dd503bf9bf683a695
    eigen/3.3.7
        ID: 5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9
    fable/0.18.0-rc4@cloe/develop
        ID: 28e8909fd0e5c5a6d409867acf35d1334f2902b9
    [...]
    zlib/1.2.11
        ID: 85d958111e9ae25b990062d9726ea88cea5a01d1

Much of the output has been replaced with the snip ``[...]`` to keep the
output short.

.. note::
   Notice how each package has a name, version, user and channel:
   ``name/version@user/channel``. When Conan builds a package, it derives
   a unique ID for the built package, based on package options, platform,
   architecture, compiler used, build type, dependencies, and other properties.
   This allows multiple binaries to be associated with a single package
   reference.

Since Cloe and other packages are constantly in development, the specific
versions used will differ, especially as new versions are released.
Currently, all Cloe packages use the same project version. This simplifies
development, since all packages are released together. It does mean though that
there might be no difference between a plugin of one version to the next.
Once Cloe becomes more stable, we may start versioning each package
separately.

When ``cloe-engine`` is built, it depends on ``cloe-runtime`` in a specific
version. It is generally not compatible with other versions of the runtime.
Plugins must also be compiled to a specific version of ``cloe-runtime``.
That does not mean that it is not possible for ``cloe-plugin-basic/0.19.0``
to be used with ``cloe-engine/0.18.0``, but it needs to be re-compiled for
the runtime that is used with that engine.

This sounds complicated, and it is. Thankfully, Conan provides a mechanism to
manage this complexity automatically: the Conan recipe. Each Conan package is
defined by a recipe, but you can also define a set of dependencies in Conan
recipe without making that to a named package. This is what we take advantage
of.

The recommended procedure to run Cloe is to use a Conan recipe to specify the
set of packages and plugins you want to use together. Conan can ensure that
the correct packages are retrieved, built if needed, and executed in an
environment together.

.. note::
   We sometimes call this Conan recipe a Cloe execution *profile* when used
   in conjunction with the ``cloe-launch`` tool. This should not be confused
   with a Conan profile, which is something else altogether.

.. highlight:: ini

The simplest recipe is to use the ``cloe`` metapackage, and can be defined
in a simple text file::

    [requires]
    cloe/0.18.0@cloe/develop

Should you want to use a different version of a plugin, or add your own, all
you have to do is add a line to the recipe::

    [requires]
    cloe-engine/0.18.0@cloe/develop
    cloe-plugin-vtd/0.20.0@cloe/develop
    acme-cloe-plugin/1.1.0@acme/nightly

If you want to add extra logic to your recipe, such as options, you can use
a Python recipe instead of a text recipe. This functionality comes entirely
from Conan, so it sets the limits on what can and can't be done.

In the next section we will see how we can run the Cloe engine from such a
recipe.

.. _Conan: https://conan.io/

----

.. rubric:: Suggested Exercises

#. | Clone the Cloe repository, if you haven't already.
   | (Hint: See the previous section.)

#. | Build the Cloe Conan packages, if you haven't already.
   | (Hint: See the previous section.)

#. | Have a look at the ``cloe`` Conan recipe.
   | (Hint: This is the ``conanfile.py`` file in the repository root.)

#. | Inspect the dependency graph of the ``cloe-engine`` package.
   | (Hint: You need to specify the full reference, ``cloe-engine/VERSION@cloe/develop``.)
