# syntax = docker/dockerfile:1.2
# Dockerfile
#
# This dockerfile simply adds the sources of ~/.conan/data/vtd/.../ so that
# they can be mounted separately in Docker builds. This keeps image sizes down.
#
# Build the image like this:
#
#   docker build -t cloe/vtd-conan-package:2022.3 Dockerfile vires
#
# And use it in another Dockerfile (with buildkit) like this:
#
#   RUN --mount=type=bind,target=/root/.conan/vtd/2022.3/cloe-restricted/stable,from=cloe/vtd-conan-package:2022.3
#
# Note, you can create this image even if you don't have anything in the vires
# directory. It will result in an empty image, but it will allow the other
# Docker builds to continue.
#
FROM ubuntu:20.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,id=ubuntu-20.04-cache,target=/var/cache/apt \
    --mount=type=cache,id=ubuntu-20.04-lib,target=/var/lib/apt \
    apt-get update && \
    apt-get install --no-install-recommends -y \
        build-essential \
        file \
        freeglut3 \
        libaa1 \
        libbsd0 \
        libexpat1 \
        libasn1-8-heimdal \
        libasound2 \
        libasyncns0 \
        openssh-server \
        nfs-common \
        mesa-utils \
        xfonts-75dpi \
        mesa-common-dev \
        libatk1.0-0 \
        libaudio2 \
        libcaca0 \
        libcairo2 \
        libdatrie1 \
        libdv4 \
        libdvdnav4 \
        libdvdread7 \
        libegl1-mesa \
        libflac8 \
        libfontconfig1 \
        libfreetype6 \
        libgdk-pixbuf2.0-0 \
        libgl1-mesa-glx \
        libglib2.0-0 \
        libglib2.0-dev \
        libglu1-mesa \
        libgpm2 \
        libgraphite2-3 \
        libgsm1 \
        libgssapi3-heimdal \
        libgtk2.0-0 \
        libharfbuzz0b \
        libhcrypto4-heimdal \
        libheimbase1-heimdal \
        libheimntlm0-heimdal \
        libhx509-5-heimdal \
        libice6 \
        libjack-jackd2-0 \
        libjpeg-turbo8 \
        libkrb5-26-heimdal \
        libldap-2.4-2 \
        libldb2 \
        liblzo2-2 \
        libmp3lame0 \
        libogg0 \
        liborc-0.4-0 \
        libpango-1.0-0 \
        libpangocairo-1.0-0 \
        libpangoft2-1.0-0 \
        libpixman-1-0 \
        libpulse0 \
        libqt5network5 \
        libqt5script5 \
        libqt5sql5 \
        libqt5svg5 \
        libqt5xml5 \
        libqt5xmlpatterns5 \
        libqt5core5a \
        libqt5gui5 \
        libroken18-heimdal \
        libsasl2-2 \
        libsdl1.2debian \
        libsm6 \
        libsmbclient \
        libsndfile1 \
        libspeex1 \
        libtalloc2 \
        libtdb1 \
        libtevent0 \
        libthai0 \
        libtheora0 \
        libusb-0.1-4 \
        libvorbis0a \
        libvorbisenc2 \
        libwbclient0 \
        libwind0-heimdal \
        libwrap0 \
        libx11-6 \
        libxau6 \
        libxcb-render0 \
        libxcb-shm0 \
        libxcb1 \
        libxcomposite1 \
        libxcursor1 \
        libxdamage1 \
        libxdmcp6 \
        libxext6 \
        libxfixes3 \
        libxi6 \
        libxinerama1 \
        libxrandr2 \
        libxrender1 \
        libxss1 \
        libxt6 \
        libxv1 \
        libxvidcore4 \
        libxvmc1 \
        libxxf86dga1 \
        libxxf86vm1 \
        netcat \
        patchelf \
        python3-pip \
        python3-setuptools \
        samba-libs \
        tcsh \
        xterm \
        wget \
        libncursesw5 \
        libpng16-16 \
        libwebp6 \
        libffi7\
        zlib1g-dev && \
        rm -rf /var/lib/apt/lists/*
        
RUN wget 'https://sourceforge.net/projects/libpng/files/libpng12/1.2.59/libpng-1.2.59.tar.xz' && \
    tar Jxfv libpng-1.2.59.tar.xz && \
    cd libpng-1.2.59 && \
    ./configure && \
    make && \
    cp .libs/libpng12.so.0.59.0 /lib/x86_64-linux-gnu/ && \
    cp -d .libs/libpng12.so.0 /lib/x86_64-linux-gnu/ && \
    rm -rf libpng-1.2.59

RUN pip3 install --upgrade pip && \
    pip3 install "conan<2.0.0"

RUN conan profile new --detect default && \
    conan profile update settings.compiler.libcxx=libstdc++11 default

ENV CONAN_NON_INTERACTIVE=yes

WORKDIR /vtd
COPY dl ./dl
COPY conanfile.py ./
COPY libdeps_pruned.txt ./
COPY entrypoint.sh ./
COPY setups ./setups
ARG PACKAGE_FQN=vtd/2022.3@cloe-restricted/stable
RUN conan create conanfile.py ${PACKAGE_FQN} && \
    conan remove -b -s -f vtd && \
    find /root/.conan/data -type d -name "dl" -exec rm -rf {} +

FROM ubuntu:20.04
COPY --from=build /root/.conan/data/vtd /vtd
RUN ln -s /vtd/*/*/*/package/* /VTD
COPY --from=build /vtd/entrypoint.sh ./
COPY --from=build /vtd/setups /vtd_setups
RUN chmod +x entrypoint.sh 
