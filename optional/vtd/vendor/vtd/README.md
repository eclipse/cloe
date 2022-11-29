Conan VTD Package
=================

NOTE: IT IS NOT RECOMMENDED TO USE ANY OF THE CONAN BUILD TARGETS DIRECTLY,
SUCH AS `make package` OR `make all`!

In order to provide a VTD Conan package that is as indepedent and
"cross-platform" as possible, several system libraries are bundled with the
package during creation. In order for this to work, they actually have to be
available on the system that is building them. Since this is unlikely to be the
case, it is recommended to build the Docker image and copy the "vtd" package
from the container once complete.

Additionally, the Dockerfile in `optional/vtd/` requires the
`cloe/vtd-conan-package` image generated here to work. This image can then be
used to mount at build- or run-time the VTD distribution, which speeds up build
times and keeps image sizes down.

In principle, all you need to do is run `make docker`, but first make sure you
fulfill the requirements.

Requirements
------------

VTD is a proprietary application from [MSC Software](https://www.mscsoftware.com/product/virtual-test-drive).
In order to use the `cloe-plugin-vtd` simulator binding, you will need
a license to use VTD and you will need the VTD 2.2.0 sources.
Please contact MSC Software for this.

In order to build this package, you will need to place the required source TGZ
files in the `dl/` directory.

If these are not available, this package will be skipped at the export stage.

Building the Image
------------------

Create `dl` directory and link or copy the source `.tgz` files:
Make sure to export `VTD_TGZ_DIR` to the actual directory where they are stored.

    export VTD_TGZ_DIR=~/vtd
    mkdir dl
    cd dl
    ln "${VTD_TGZ_DIR}/vtd.2.2.0.Base.20181231.tgz" ./
    ln "${VTD_TGZ_DIR}/vtd.2.2.0.addOns.OSI.20210120.tgz" ./
    ln "${VTD_TGZ_DIR}/vtd.2.2.0.addOns.ROD.Base.20190313.tgz" ./

Build the `cloe/vtd-conan-package` Docker image:

    make docker

Extracting the Conan Packages
-----------------------------

If you already have the `vtd/2.2.0@cloe-restricted/stable` package and you want
to "refresh" it, it's recommended to delete the local copy first:

    conan remove vtd/2.2.0@cloe-restricted/stable

Normally, there is no need to re-create the package though.

Make sure you have the `cloe/vtd-conan-package:2.2.0` Docker image available;
see the previous section if you need to build it. Then mount *your* conan data
home (by default, `~/.conan/data`) into the docker container.

    mkdir -p ~/.conan/data/vtd
    touch ~/.conan/data/.user_id.tmp
    docker run --rm -v ~/.conan/data:/mnt cloe/vtd-conan-package:2.2.0 bash -c \
        "cp -a /vtd/* /mnt/vtd/ && chown -R --reference=/mnt/.user_id.tmp /mnt/vtd"
    rm ~/.conan/data/.user_id.tmp

For your convenience, this is also available as a make target:

    make extract-to-host
