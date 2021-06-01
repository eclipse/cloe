Conan VTD Package
=================

In order to build this package and the `vtd-api` package next to this package, you will
need to place the required source TGZ files in the `dl/` directory.

If these are not available, this package will be skipped at the export stage.

For the Docker images in `dist/docker` to work, you should create the
`cloe/vtd-conan-package` Docker image:

    make docker

This also requires, as above, that the required source TGZ files are in the `dl/`
directory.

This image can then be used to mount at build- or run-time the VTD distribution,
which speeds up build times and keeps image sizes down.
