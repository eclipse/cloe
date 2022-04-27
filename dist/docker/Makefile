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
KEEP_SOURCES :=

DOCKER_BUILD_ARGS += ${DOCKER_NETWORK}
DOCKER_BUILD_ARGS += --build-arg PROJECT_VERSION=${PROJECT_VERSION}

ifeq ($(shell [ -f setup.sh ] && echo "true"),true)
DOCKER_BUILD_ARGS += --secret id=setup,src=setup.sh
endif

ifeq (${DEBUG},1)
DOCKER_BUILD_ARGS += --progress=plain
endif

ifneq (${BUILD_TYPE},)
DOCKER_BUILD_ARGS += --build-arg BUILD_TYPE=${BUILD_TYPE}
endif

ifneq (${VENDOR_TARGET},)
DOCKER_BUILD_ARGS += --build-arg VENDOR_TARGET=${VENDOR_TARGET}
endif

ifneq (${PACKAGE_TARGET},)
DOCKER_BUILD_ARGS += --build-arg PACKAGE_TARGET=${PACKAGE_TARGET}
endif

ifeq (${WITH_VTD},1)
DOCKER_IMAGE := ${DOCKER_IMAGE}-vtd
VTD_VERSION := $(shell make --no-print-directory -C ${PROJECT_ROOT}/vendor/vtd info-version 2>/dev/null)
VTD_IMAGE   := cloe/vtd-conan-package:${VTD_VERSION}
DOCKER_BUILD_ARGS += --build-arg WITH_VTD=${WITH_VTD}
DOCKER_BUILD_ARGS += --build-arg VTD_IMAGE=${VTD_IMAGE}
endif

ifeq (${WITH_VTD},1)
DOCKER_IMAGE := ${DOCKER_IMAGE}-with-src
DOCKER_BUILD_ARGS += --build-arg KEEP_SOURCES=1
endif

DOCKER_ARGS := ${DOCKER_BUILD_ARGS}

.PHONY: help
.SILENT: help
.DEFAULT: help
help:
	echo "Usage: make <target>"
	echo
	echo "Available build targets:"
	echo "  all            to build all stable docker images"
	echo "  ubuntu-20.04   to build the Ubuntu 20.04 image"
	echo "  ubuntu-18.04   to build the Ubuntu 18.04 image"
	echo "  ubuntu-16.04   to build the Ubuntu 18.04 image"
	echo "  archlinux      to build the Archlinux image"
	echo
	echo "Available system targets:"
	echo "  purge-current  to purge and prune current docker versions"
	echo "  purge-all      to purge and prune all docker versions"
	echo
	echo "Configuration:"
	echo "  IMAGE_BASE:     ${IMAGE_BASE}"
	echo "  IMAGE_VERSION:  ${IMAGE_VERSION}"
	echo "  DOCKER_CONTEXT: ${DOCKER_CONTEXT}"
	echo "  DOCKER_ARGS:    ${DOCKER_ARGS}"

.PHONY: all ubuntu-16.04 ubuntu-18.04 ubuntu-20.04 archlinux
all: ubuntu-20.04 ubuntu-18.04 ubuntu-16.04

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

release:
	@test -f setup.sh || echo 'Error: require setup.sh for user authentication'
	for dist in ubuntu-16.04 ubuntu-18.04 ubuntu-20.04; do \
		${DOCKER} run --rm \
			$(subst --build-arg,-e,${DOCKER_NETWORK}) \
			--mount "type=bind,source=$(CURDIR)/setup.sh,target=/root/setup.sh" \
			${DOCKER_IMAGE}-$${dist} \
			bash -c '. /root/setup.sh && conan upload --all -no -c "*"'; \
	done

.PHONY: purge-current purge-all
purge-current:
	docker rmi $$(docker images --format '{{.Repository}}:{{.Tag}}' | grep '${DOCKER_IMAGE}')

purge-all:
	docker rmi $$(docker images --format '{{.Repository}}:{{.Tag}}' | grep '${IMAGE_BASE}:')
