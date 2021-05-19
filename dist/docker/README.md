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
  alpine        to build the Alpine image

Configuration:
  IMAGE_BASE:     cloe/cloe-engine
  IMAGE_VERSION:  0.18.0-nightly
  CONAN_REMOTE:   https://artifactory.example.com/artifactory/api/conan/cloe
  DOCKER_CONTEXT: ../..
  DOCKER_ARGS:     --network=host
                   --build-arg https_proxy=http://127.0.0.1:3128/
                   --build-arg http_proxy=http://127.0.0.1:3128/
                   --build-arg no_proxy=localhost,127.0.0.1,127.*,172.*,192.168.*,10.*
                   --build-arg CONAN_REMOTE=https://artifactory.example.com/artifactory/api/conan/cloe
                   --build-arg CONAN_REMOTE_VERIFY_SSL=True
                   --build-arg CONAN_LOGIN_USERNAME=cloebuilder
                   --build-arg CONAN_PASSWORD=secret
                   --build-arg WITH_VTD=0
                   --build-arg VI_LIC_SERVER=license-server.example.com
                   --build-arg PACKAGE_TARGET=package-select
```

The output has been slightly formatted to make the Docker build arguments a
little more obvious. The following build arguments are available:

- `CONAN_REMOTE`: optional string; defaults to conan-center URL.
- `CONAN_REMOTE_VERIFY_SSL`: optional, one of `True` and `False`; defaults to `True`.
- `CONAN_LOGIN_USERNAME`: optional username for authenticating with remote.
- `CONAN_PASSWORD`: optional password, required if username specified.
- `WITH_VTD`: optional, one of `0` and `1`; defaults to `0`.
- `VI_LIC_SERVER`: optional hostname of VTD license server.
- `PACKAGE_TARGET`: optional, one of `package`, `package-select`, and `package-all`;
  defaults to `package-select`.

You can specify these arguments on the command line when calling make, or you
can specify them in a `.env` file, which is exempt from version control.
It is recommended to specify the Conan build arguments in the `.env` file, and
the other arguments on the command line. Because of the way the .env file is
read by Make, it is important to not use any quotation marks around the values.

Because Docker images may be built in environments that have a proxy running,
the Makefile will automatically add the proxy variables if they are are
detected in the host environment.
