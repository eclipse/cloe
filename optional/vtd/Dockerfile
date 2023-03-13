# syntax = docker/dockerfile:1.4.3
# Dockerfile
#
# This file acts as a Docker recipe for building cloe/cloe-engine-with-vtd
# on Ubuntu. It uses the base image from Cloe to reduce overhead.
#
# Generally, the cloe-engine image looks like:
#
#   cloe/cloe-engine:0.19.0-ubuntu-18.04
#
# The correct reference should be assembled before building the image.
#
# Note to maintainer:
#   Make sure you repeat any ARG required after every FROM statement.
ARG UBUNTU_VERSION
ARG PROJECT_VERSION
ARG CLOE_IMAGE=cloe/cloe-engine:${PROJECT_VERSION}-ubuntu-${UBUNTU_VERSION}

# This is a work-around to not being able to use variables in RUN --mount=from:
# If you want to use VTD in this image, you need to specify the Docker image
# containing the distribution that can be mounted at /root/.conan/data/
ARG VTD_IMAGE=scratch
FROM ${VTD_IMAGE} AS vtd
WORKDIR /vtd

FROM ${CLOE_IMAGE} AS stage-vendor
ARG KEEP_SOURCES=0
ARG VENDOR_TARGET="export-vendor download-vendor"

WORKDIR /cloe/optional/vtd
SHELL ["/bin/bash", "-c"]

# Download vendor packages:
COPY vendor /cloe/optional/vtd/vendor
COPY Makefile /cloe/optional/vtd
COPY conanfile.py /cloe/optional/vtd
RUN --mount=type=cache,target=/ccache \
    --mount=type=secret,target=/root/setup.sh,id=setup,mode=0400 \
    --mount=type=bind,target=/root/.conan/data/vtd,source=/vtd,from=vtd,rw \
    if [ -r /root/setup.sh ]; then . /root/setup.sh; fi && \
    make ${VENDOR_TARGET} && \
    # Clean up:
    if [ ${KEEP_SOURCES} -eq 0 ]; then \
        find /root/.conan/data -name dl -type d -maxdepth 5 -exec rm -r {} + && \
        conan remove \* -s -b -f; \
    else \
        conan remove \* -b -f; \
    fi

# Build cloe-plugin-vtd package:
FROM stage-vendor AS stage-build
ARG PROJECT_VERSION
ARG PACKAGE_TARGET="export smoketest-deps"

COPY . /cloe/optional/vtd
RUN --mount=type=cache,target=/ccache \
    --mount=type=secret,target=/root/setup.sh,id=setup,mode=0400 \
    --mount=type=bind,target=/root/.conan/data/vtd,source=/vtd,from=vtd,rw \
    if [ -r /root/setup.sh ]; then . /root/setup.sh; fi && \
    echo "${PROJECT_VERSION}" > VERSION && \
    make ${PACKAGE_TARGET} && \
    # Clean up:
    if [ ${KEEP_SOURCES} -eq 0 ]; then \
        find /root/.conan/data -name dl -type d -maxdepth 5 -exec rm -r {} + && \
        conan remove \* -s -b -f; \
    else \
        conan remove \* -b -f; \
    fi
