# Overrides will be preserved in Makefile.package.
override SOURCE_DIR := src
override PACKAGE_CHANNEL := cloe/stable
override CLEAN_SOURCE_DIR := true

include ../../Makefile.package

export:
	# Only try to export the vtd sources if they are actually there.
	@( \
		file=$$(cat conanfile.py \
		| sed -rne 's/^.*_archive_base.*"(.*\.tgz)"$$/\1/p'); \
		if [ ! -f $$file ]; then \
			echo "Missing source tgz: $$file"; \
			echo "Skipping export of: ${PACKAGE_FQN}"; \
			return 1; \
		fi \
	) && conan export . ${PACKAGE_FQN} || true
