# Usage: make [options] <target>
#

PROJECT_ROOT := ../..
PROJECT_VERSION := $(shell make --no-print-directory -C ${PROJECT_ROOT} -f Makefile.package info-version)

DOCKER := DOCKER_BUILDKIT=1 docker

DOCKER_NETWORK := \
	$(shell \
		if [ -n $$https_proxy ]; then \
			echo " --network=host"; \
			echo " --build-arg https_proxy=\"$$https_proxy\""; \
			echo " --build-arg http_proxy=\"$$http_proxy\""; \
			echo " --build-arg no_proxy=\"$$no_proxy\""; \
		fi \
	)

IMAGE_BASE := cloe/cloe-engine
IMAGE_VERSION := ${PROJECT_VERSION}
DOCKER_IMAGE := ${IMAGE_BASE}:${IMAGE_VERSION}
DOCKER_CONTEXT := ${PROJECT_ROOT}

# Default build arguments
VENDOR_TARGET :=
PACKAGE_TARGET :=
BUILD_TYPE :=
WITH_VTD :=

DOCKER_BUILD_ARGS += ${DOCKER_NETWORK}
DOCKER_BUILD_ARGS += --progress=plain
DOCKER_BUILD_ARGS += --build-arg PROJECT_VERSION=${PROJECT_VERSION}

ifeq ($(shell [ -f setup.sh ] && echo "true"),true)
DOCKER_BUILD_ARGS += --secret id=setup,src=setup.sh
endif

ifneq (${BUILD_TYPE},)
DOCKER_BUILD_ARGS += --build-arg BUILD_TYPE=${BUILD_TYPE}
endif

ifneq (${VENDOR_TARGET},)
DOCKER_BUILD_ARGS += --build-arg PACKAGE_TARGET=${PACKAGE_TARGET}
endif

ifneq (${PACKAGE_TARGET},)
DOCKER_BUILD_ARGS += --build-arg PACKAGE_TARGET=${PACKAGE_TARGET}
endif

ifeq (${WITH_VTD},1)
DOCKER_IMAGE := ${DOCKER_IMAGE}-vtd
DOCKER_BUILD_ARGS += --build-arg WITH_VTD=${WITH_VTD}
endif

DOCKER_ARGS := ${DOCKER_BUILD_ARGS}

.PHONY: help
.SILENT: help
.DEFAULT: help
help:
	echo "Usage: make <target>"
	echo
	echo "Available targets:"
	echo "  all           to build all stable docker images"
	echo "  ubuntu-20.04  to build the Ubuntu 20.04 image"
	echo "  ubuntu-18.04  to build the Ubuntu 18.04 image"
	echo "  ubuntu-16.04  to build the Ubuntu 18.04 image"
	echo "  archlinux     to build the Archlinux image"
	echo "  vires         to build the required VTD sources image"
	echo
	echo "Configuration:"
	echo "  IMAGE_BASE:     ${IMAGE_BASE}"
	echo "  IMAGE_VERSION:  ${IMAGE_VERSION}"
	echo "  CONAN_REMOTE:   ${CONAN_REMOTE}"
	echo "  DOCKER_CONTEXT: ${DOCKER_CONTEXT}"
	echo "  DOCKER_ARGS:    ${DOCKER_ARGS}"

.PHONY: all ubuntu-16.04 ubuntu-18.04 ubuntu-20.04 archlinux vires
all: ubuntu-20.04 ubuntu-18.04 ubuntu-16.04 archlinux

ubuntu-16.04: Dockerfile.xenial
	@echo ===================================================================================
	${DOCKER} build -f $< ${DOCKER_ARGS} -t ${DOCKER_IMAGE}-$@ ${DOCKER_CONTEXT}

ubuntu-18.04: Dockerfile.bionic
	@echo ===================================================================================
	${DOCKER} build -f $< ${DOCKER_ARGS} -t ${DOCKER_IMAGE}-$@ ${DOCKER_CONTEXT}

ubuntu-20.04: Dockerfile.focal
	@echo ===================================================================================
	${DOCKER} build -f $< ${DOCKER_ARGS} -t ${DOCKER_IMAGE}-$@ ${DOCKER_CONTEXT}

archlinux: Dockerfile.archlinux
	@echo ===================================================================================
	${DOCKER} build -f $< ${DOCKER_ARGS} -t ${DOCKER_IMAGE}-$@ ${DOCKER_CONTEXT}

# The VTD image is required for the other builds, but can be empty if you don't
# have access to actual VTD sources.
VTD_VERSION := $(shell make --no-print-directory -C ${PROJECT_ROOT}/vendor/vtd info-version 2>/dev/null)
VTD_IMAGE   := cloe/vtd-sources:${VTD_VERSION}
VTD_CONTEXT := ${PROJECT_ROOT}/vendor/vtd/vires
vires: Dockerfile.vires
	@echo ===================================================================================
	[ -d ${VTD_CONTEXT} ] || mkdir ${VTD_CONTEXT}
	${DOCKER} build -f $< ${DOCKER_ARGS} -t ${VTD_IMAGE} ${VTD_CONTEXT}
