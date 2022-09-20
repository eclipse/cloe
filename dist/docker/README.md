Cloe Docker Images
==================

This directory contains Dockerfiles to build Cloe Conan packages in various
environments. We use a Makefile to automate the build process as much as
possible. Run `make help` to see a list of the available targets.

```console
$ make help
Usage: make <target>

Available targets:
  all           to build all stable docker images
  ubuntu-20.04  to build the Ubuntu 20.04 image
  ubuntu-18.04  to build the Ubuntu 18.04 image
  ubuntu-16.04  to build the Ubuntu 18.04 image
  archlinux     to build the Archlinux image
  vires         to build the required VTD sources image

Configuration:
  IMAGE_BASE:     cloe/cloe-engine
  IMAGE_VERSION:  0.18.0-nightly
  CONAN_REMOTE:   https://artifactory.example.com/artifactory/api/conan/cloe
  DOCKER_CONTEXT: ../..
  DOCKER_ARGS:     --network=host
                   --build-arg https_proxy=http://127.0.0.1:3128/
                   --build-arg http_proxy=http://127.0.0.1:3128/
                   --build-arg no_proxy=localhost,127.0.0.1,127.*,172.*,192.168.*,10.*
                   --progress=plain
                   --secret id=setup,src=setup.sh
```

The output has been slightly formatted to make the Docker build arguments a
little more obvious.

Because Docker images may be built in environments that have a proxy running,
the Makefile will automatically add the proxy variables if they are are
detected in the host environment.

The following build arguments are available and should be specified on the
command line when calling make:

- `WITH_VTD`: optional, one of `0` and `1`; defaults to `0`.
- `BUILD_TYPE`: optional, one of `Release`, `RelWithDebInfo`, `Debug`;
  defaults to `RelWithDebInfo`.
- `VENDOR_TARGET`: optional; defaults to `export-vendor download-vendor`.
- `PACKAGE_TARGET`: optional, one of `package`, `package`,
  `package-select`, and `package-all`; defaults to `package`.

If you want to use a different Conan remote from the default, you need to
copy `setup.sh.example` to `setup.sh` and modify the values to match your
environment.

Note that this build requires the use of docker buildx, which has been
available for some time. This allows us to mount secrets in at build time
and also speeds up the build by the strategic use of caches.
