# Overrides will be preserved in Makefile.package.
override SOURCE_DIR := src
override PACKAGE_CHANNEL := cloe-restricted/stable
override CLEAN_SOURCE_DIR := true

PROJECT_ROOT := ../../../..
include ${PROJECT_ROOT}/Makefile.package

# Override from Makefile.package:
export:
	# Only try to export the vtd sources if they are actually there.
	@( \
		file=$$(cat conanfile.py | sed -rne 's/^.*_archive_base.*"(.*\.tgz)"$$/\1/p'); \
		if [ ! -f $$file ]; then \
			echo "Missing source tgz: $$file"; \
			echo "Skipping export of: ${PACKAGE_FQN}"; \
			exit 1; \
		fi \
	) && conan export . ${PACKAGE_FQN} || true

# Override from Makefile.package:
download:
	# Disabled. Please make sure that vtd is in your Conan cache.

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

DOCKER_IMAGE := cloe/vtd-conan-package
DOCKER_TAG := ${PACKAGE_VERSION}
DOCKER_VOLUME := conan-data-vtd

help::
	$(call print_help_section, "Available docker targets")
	$(call print_help_target, docker, "build docker image containing Conan vtd package")
	$(call print_help_target, extract-to-host, "extract vtd Conan package from image to host filesystem")
	$(call print_help_target, extract-to-volume, "extract vtd Conan package from image to volume for use as side-car")
	echo
	$(call print_help_subsection, "Docker configuration")
	$(call print_help_define_align, DOCKER_IMAGE, ${DOCKER_IMAGE})
	$(call print_help_define_align, DOCKER_TAG, ${DOCKER_TAG})
	$(call print_help_define_align, DOCKER_VOLUME, ${DOCKER_VOLUME})

.PHONY: docker
docker:
	${DOCKER} build ${DOCKER_NETWORK} -t ${DOCKER_IMAGE}:${DOCKER_TAG} --build-arg PACKAGE_FQN=${PACKAGE_FQN} .

.PHONY: extract-to-host
extract-to-host:
	touch ~/.conan/data/.user_id.tmp
	docker run --rm -v ~/.conan/data:/mnt ${DOCKER_IMAGE}:${DOCKER_TAG} bash -c \
		"cp -a /vtd/ /mnt/vtd && chown -R --reference=/mnt/.user_id.tmp /mnt/vtd"
	rm ~/.conan/data/.user_id.tmp

.PHONY: extract-to-volume
extract-to-volume:
	docker run --rm -v ${DOCKER_VOLUME}:/mnt ${DOCKER_IMAGE}:${DOCKER_TAG} bash -c "cp -a /vtd/* /mnt/"

.PHONY: list-vtd-volume
list-vtd-volume:
	docker run --rm -v ${DOCKER_VOLUME}:/mnt ubuntu bash -c "ls -a /mnt"

.PHONY: remove-vtd-volume
remove-vtd-volume:
	docker volume rm ${DOCKER_VOLUME}
