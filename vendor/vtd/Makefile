# Overrides will be preserved in Makefile.package.
override SOURCE_DIR := src
override PACKAGE_CHANNEL := cloe-restricted/stable
override CLEAN_SOURCE_DIR := true

include ../../Makefile.package

export:
	# Only try to export the vtd sources if they are actually there.
	@( \
		file=$$(cat conanfile.py | sed -rne 's/^.*_archive_base.*"(.*\.tgz)"$$/\1/p'); \
		if [ ! -f $$file ]; then \
			echo "Missing source tgz: $$file"; \
			echo "Skipping export of: ${PACKAGE_FQN}"; \
			return 1; \
		fi \
	) && conan export . ${PACKAGE_FQN} || true

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

.PHONY: docker
docker:
	${DOCKER} build \
		${DOCKER_NETWORK} \
		-t cloe/vtd-conan-package:${PACKAGE_VERSION} \
		--build-arg PACKAGE_FQN=${PACKAGE_FQN} \
		.

.PHONY: extract-conan-package
extract-conan-package:
	touch ~/.conan/data/.user_id.tmp
	docker run --rm -v ~/.conan/data:/mnt cloe/vtd-conan-package:2.2.0 bash -c \
		"cp -a /vtd/ /mnt/vtd && chown -R --reference=/mnt/.user_id.tmp /mnt/vtd"
	rm ~/.conan/data/.user_id.tmp
