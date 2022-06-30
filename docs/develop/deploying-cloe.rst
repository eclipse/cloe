Deploying Cloe
==============

It is recommended to distribute Cloe binary packages via your Conan remote
repository. To understand the workflow and upload packages to a local Conan
server for testing, refer to the provided dockerized Conan server example
(s. ``tools/conan-server/README.md``) and the `Conan package upload`_
documentation.

The Cloe package release process has been automated for multiple Linux
distributions. Please make sure you understand the instructions in
``dist/docker/README.md``, particularly with respect to the provided
``dist/docker/setup.sh.example`` file.

You may start the release process from the top-level directory like so::

  make docker-release

This will build and test all Cloe packages in multiple Linux environments and
upload the binaries to the Conan remote repository specified in
``dist/docker/setup.sh``.


.. _Conan package upload: https://docs.conan.io/en/latest/uploading_packages.html
