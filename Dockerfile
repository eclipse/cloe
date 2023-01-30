# syntax = docker/dockerfile:1.4.3
# Dockerfile.ubuntu
#
# This file acts partly as a Docker recipe for building Cloe on Ubuntu.
# It is known to work with Ubuntu 18.04, 20.04, and 22.04.
#
# If you are behind a proxy, make sure to pass in the respective HTTP_PROXY,
# HTTPS_PROXY, and NO_PROXY variables.
#
# Note to maintainer:
#   Make sure you repeat any ARG required after every FROM statement.
ARG UBUNTU_NAME=ubuntu
ARG UBUNTU_VERSION=20.04
ARG UBUNTU_IMAGE=${UBUNTU_NAME}:${UBUNTU_VERSION}
FROM ${UBUNTU_IMAGE} AS stage-setup-system
ARG UBUNTU_VERSION

# Install System Packages
#
# These packages are required for building and testing Cloe.
COPY Makefile.help  /cloe/Makefile.help
COPY Makefile.setup /cloe/Makefile.setup
RUN rm -f /etc/apt/apt.conf.d/docker-clean; echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache
RUN --mount=type=cache,id=ubuntu-${UBUNTU_VERSION}-cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=ubuntu-${UBUNTU_VERSION}-lib,target=/var/lib/apt,sharing=locked \
    apt-get update && \
    apt-get install -y make ccache locales libbsd0 && \
    make -f /cloe/Makefile.setup \
        DEBIAN_FRONTEND=noninteractive \
        APT_ARGS="--no-install-recommends -y" \
        install-system-deps && \
    locale-gen

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV CCACHE_DIR=/ccache
ENV PATH=/usr/lib/ccache:$PATH

RUN pip3 install --upgrade pip && \
    make -f /cloe/Makefile.setup \
        PIP_INSTALL_ARGS="" \
        install-python-deps

# Install and Setup Conan
#
# You may not want to use the default Conan remote (conancenter), so we use
# whatever is stored in the build arguments CONAN_REMOTE.
#
# If you need to login to a Conan remote, make use of the setup.sh file, which
# will be sourced for every run argument that uses the conan command.
#
# The following profiles are available: default, cloe-release, cloe-debug
FROM stage-setup-system AS stage-setup-conan
ARG CONAN_PROFILE=cloe-release
ENV CONAN_NON_INTERACTIVE=yes

COPY dist/conan /cloe/dist/conan
RUN make -f /cloe/Makefile.setup setup-conan && \
    conan config set general.default_profile=${CONAN_PROFILE} && \
    # NOTE: Boost 1.69 has a bug (fixed in 74fb0a26099bc51d717f5f154b37231ce7df3e98) that
    # prevents GCC >= 11 from interoperation, so disable the server for this case.
    # (The server feature depends on cloe-oak, which *currently* depends on Boost <= 1.69.)
    if [ $(gcc -dumpversion) -ge 11 ]; then \
        conan profile update options.cloe-engine:server=False ${CONAN_PROFILE}; \
    fi

# Build and Install Cloe
#
# All common processes are made easy to apply by writing target recipes in the
# Makefile at the root of the repository. This also acts as a form of
# documentation.
FROM stage-setup-conan AS stage-vendor
ARG VENDOR_TARGET="export-vendor download-vendor"
ARG KEEP_SOURCES=0

WORKDIR /cloe

# Download or build dependencies:
COPY vendor /cloe/vendor
COPY Makefile.package /cloe
COPY Makefile.all /cloe
RUN --mount=type=cache,target=/ccache \
    --mount=type=secret,target=/root/setup.sh,id=setup,mode=0400 \
    if [ -r /root/setup.sh ]; then . /root/setup.sh; fi && \
    make -f Makefile.all ${VENDOR_TARGET} && \
    # Clean up:
    conan user --clean && \
    if [ ${KEEP_SOURCES} -eq 0 ]; then \
        find /root/.conan/data -name dl -type d -maxdepth 5 -exec rm -r {} + && \
        conan remove \* -s -b -f; \
    else \
        conan remove \* -b -f; \
    fi

# Build Cloe:
FROM stage-vendor AS stage-build
ARG PROJECT_VERSION=unknown
ARG PACKAGE_TARGET="export smoketest-deps"
ARG KEEP_SOURCES=0

COPY . /cloe
RUN --mount=type=cache,target=/ccache \
    --mount=type=secret,target=/root/setup.sh,id=setup,mode=0400 \
    if [ -r /root/setup.sh ]; then . /root/setup.sh; fi && \
    echo "${PROJECT_VERSION}" > /cloe/VERSION && \
    make ${PACKAGE_TARGET} && \
    # Clean up:
    conan user --clean && \
    if [ ${KEEP_SOURCES} -eq 0 ]; then \
        find /root/.conan/data -name dl -type d -maxdepth 5 -exec rm -r {} + && \
        conan remove \* -s -b -f; \
    else \
        conan remove \* -b -f; \
    fi
