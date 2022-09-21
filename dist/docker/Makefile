# Usage: make [options] <target>
#

PROJECT_ROOT := ../..
PROJECT_VERSION := $(shell make --no-print-directory -C ${PROJECT_ROOT} -f Makefile.package info-version)

include ${PROJECT_ROOT}/Makefile.help

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
CONAN_PROFILE :=
KEEP_SOURCES :=

DOCKER_BUILD_ARGS += ${DOCKER_NETWORK}
DOCKER_BUILD_ARGS += --build-arg PROJECT_VERSION=${PROJECT_VERSION}

ifeq ($(shell [ -f setup.sh ] && echo "true"),true)
DOCKER_BUILD_ARGS += --secret id=setup,src=setup.sh
endif

ifneq (${CONAN_PROFILE},)
DOCKER_BUILD_ARGS += --build-arg CONAN_PROFILE=${CONAN_PROFILE}
endif

ifneq (${VENDOR_TARGET},)
DOCKER_BUILD_ARGS += --build-arg VENDOR_TARGET=${VENDOR_TARGET}
endif

ifneq (${PACKAGE_TARGET},)
DOCKER_BUILD_ARGS += --build-arg PACKAGE_TARGET=${PACKAGE_TARGET}
endif

ifeq (${KEEP_SOURCES},1)
DOCKER_IMAGE := ${DOCKER_IMAGE}-with-src
DOCKER_BUILD_ARGS += --build-arg KEEP_SOURCES=1
endif

DOCKER_ARGS := ${DOCKER_BUILD_ARGS}

.PHONY: help
.SILENT: help
.DEFAULT: help
help:
	$(call print_help_usage)
	echo
	$(call print_help_section, "Available build targets")
	$(call print_help_target, all, "build all stable docker images")
	$(call print_help_target, ubuntu-22.04, "build the Ubuntu 22.04/jammy image")
	$(call print_help_target, ubuntu-20.04, "build the Ubuntu 20.04/focal image")
	$(call print_help_target, ubuntu-18.04, "build the Ubuntu 18.04/bionic image")
	$(call print_help_target, archlinux, "build the Archlinux image")
	echo
	$(call print_help_section, "Available system targets")
	$(call print_help_target, release, "push Conan packages from current images to remote")
	$(call print_help_target, purge-current, "purge and prune current docker versions")
	$(call print_help_target, purge-all, "purge and prune all docker versions")
	echo
	$(call print_help_section, "Configuration")
	$(call print_help_define, IMAGE_BASE, ${IMAGE_BASE})
	$(call print_help_define, IMAGE_VERSION, ${IMAGE_VERSION})
	$(call print_help_define, DOCKER_CONTEXT, ${DOCKER_CONTEXT})
	$(call print_help_define_lines, DOCKER_ARGS, ${DOCKER_ARGS})

.PHONY: all ubuntu-18.04 ubuntu-20.04 ubuntu-22.04 archlinux
all: ubuntu-22.04 ubuntu-20.04 ubuntu-18.04

ubuntu-18.04 ubuntu-20.04 ubuntu-22.04: Dockerfile.ubuntu
	@echo ===================================================================================
	${DOCKER} build -f Dockerfile.ubuntu --build-arg UBUNTU_VERSION=$(subst ubuntu-,,$@) ${DOCKER_ARGS} -t ${DOCKER_IMAGE}-$@ ${DOCKER_CONTEXT}

archlinux: Dockerfile.archlinux
	@echo ===================================================================================
	${DOCKER} build -f $< ${DOCKER_ARGS} -t ${DOCKER_IMAGE}-$@ ${DOCKER_CONTEXT}

release:
	@test -f setup.sh || echo 'Error: require setup.sh for user authentication'
	for dist in ubuntu-18.04 ubuntu-20.04 ubuntu-22.04; do \
		${DOCKER} run --rm \
			$(subst --build-arg,-e,${DOCKER_NETWORK}) \
			--mount "type=bind,source=$(CURDIR)/setup.sh,target=/root/setup.sh" \
			${DOCKER_IMAGE}-$${dist} \
			bash -c '. /root/setup.sh && conan upload --force --all -c "*"'; \
	done

.PHONY: purge-current purge-all
purge-current:
	docker rmi $$(docker images --format '{{.Repository}}:{{.Tag}}' | grep '${DOCKER_IMAGE}')

purge-all:
	docker rmi $$(docker images --format '{{.Repository}}:{{.Tag}}' | grep '${IMAGE_BASE}:')
