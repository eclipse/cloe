# Usage: make [options] <target>
#

CLOE_ROOT := ../..
CLOE_VERSION := $(shell cat ${CLOE_ROOT}/VERSION)

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
IMAGE_VERSION := ${CLOE_VERSION}
DOCKER_IMAGE := ${IMAGE_BASE}:${IMAGE_VERSION}
DOCKER_CONTEXT := ${CLOE_ROOT}

# Default build arguments
PACKAGE_TARGET := package-select
BUILD_TYPE := Release
WITH_VTD := 0
VI_LIC_SERVER :=

# Read extra build arguments from the .env file.
# The .env file is in .gitignore so the user can write their overrides there.
# This is currently primarily used so that the user can set their preferred
# Conan remote with the variables CONAN_REMOTE and CONAN_REMOTE_VERIFY_SSL.
-include .env

DOCKER_BUILD_ARGS += ${DOCKER_NETWORK}

ifneq (${CONAN_REMOTE},)
DOCKER_BUILD_ARGS += --build-arg CONAN_REMOTE="${CONAN_REMOTE}"
DOCKER_BUILD_ARGS += --build-arg CONAN_REMOTE_VERIFY_SSL="${CONAN_REMOTE_VERIFY_SSL}"
endif

ifneq (${CONAN_LOGIN_USERNAME},)
DOCKER_BUILD_ARGS += --build-arg CONAN_LOGIN_USERNAME="${CONAN_LOGIN_USERNAME}"
DOCKER_BUILD_ARGS += --build-arg CONAN_PASSWORD="${CONAN_PASSWORD}"
endif

DOCKER_BUILD_ARGS += --build-arg PACKAGE_TARGET=${PACKAGE_TARGET}

ifeq (${WITH_VTD},1)
DOCKER_IMAGE := ${DOCKER_IMAGE}-vtd
DOCKER_BUILD_ARGS += --build-arg WITH_VTD=${WITH_VTD}
DOCKER_BUILD_ARGS += --build-arg VI_LIC_SERVER="${VI_LIC_SERVER}"
endif

DOCKER_ARGS := ${DOCKER_BUILD_ARGS}

.PHONY: help
.DEFAULT: help
help:
	@echo "Usage: make <target>"
	@echo
	@echo "Available targets:"
	@echo "  all           to build all stable docker images"
	@echo "  ubuntu-20.04  to build the Ubuntu 20.04 image"
	@echo "  ubuntu-18.04  to build the Ubuntu 18.04 image"
	@echo "  ubuntu-16.04  to build the Ubuntu 18.04 image"
	@echo "  archlinux     to build the Archlinux image"
	@echo "  alpine        to build the Alpine image"
	@echo
	@echo "Configuration:"
	@echo "  IMAGE_BASE:     ${IMAGE_BASE}"
	@echo "  IMAGE_VERSION:  ${IMAGE_VERSION}"
	@echo "  CONAN_REMOTE:   ${CONAN_REMOTE}"
	@echo "  DOCKER_CONTEXT: ${DOCKER_CONTEXT}"
	@echo "  DOCKER_ARGS:    ${DOCKER_ARGS}"

.PHONY: all ubuntu-16.04 ubuntu-18.04 ubuntu-20.04 archlinux alpine
all: ubuntu-16.04 ubuntu-18.04 ubuntu-20.04 archlinux alpine

ubuntu-16.04: Dockerfile.xenial
	@echo ===================================================================================
	docker build -f $< ${DOCKER_ARGS} \
		-t ${DOCKER_IMAGE}-$@ \
		${DOCKER_CONTEXT}

ubuntu-18.04: Dockerfile.bionic
	@echo ===================================================================================
	docker build -f $< ${DOCKER_ARGS} \
		-t ${DOCKER_IMAGE}-$@ \
		${DOCKER_CONTEXT}

ubuntu-20.04: Dockerfile.focal
	@echo ===================================================================================
	docker build -f $< ${DOCKER_ARGS} \
		-t ${DOCKER_IMAGE}-$@ \
		${DOCKER_CONTEXT}

archlinux: Dockerfile.archlinux
	@echo ===================================================================================
	docker build -f $< ${DOCKER_ARGS} \
		--build-arg BUILD_FROM=archlinux \
		-t ${DOCKER_IMAGE}-$@ \
		${DOCKER_CONTEXT}

alpine: Dockerfile.alpine
	@echo ===================================================================================
	docker build -f $< ${DOCKER_ARGS} \
		--build-arg BUILD_FROM=alpine:3.12 \
		-t ${DOCKER_IMAGE}-$@ \
		${DOCKER_CONTEXT}
