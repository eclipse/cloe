# Makefile
#
# This file contains Makefile targets for the cloe project.
#

# Set the clang-format command line to use:
CLANG_FORMAT := $(shell command -v clang-format 2>/dev/null)
CLANG_FORMAT_ARGS := -style=file

AG := $(or \
	$(shell command -v rg 2>/dev/null), \
	$(shell command -v ag 2>/dev/null), \
	"grep -r" \
)

define print_header
	@printf "________________________________________"
	@printf "________________________________________\n"
	@printf ":: %s\n" ${1}
endef

CLOE_ROOT   := $(shell pwd)
CLOE_LAUNCH := PYTHONPATH="${CLOE_ROOT}/cli" python3 -m cloe_launch

# Build configuration:
BUILD_DIR       := build
LOCKFILE_SOURCE := conanfile.py
BUILD_LOCKFILE  := ${BUILD_DIR}/conan.lock
LOCKFILE_OPTION := --lockfile="${CLOE_ROOT}/${BUILD_LOCKFILE}"
INSTALL_DIR     := /usr/local
CONAN_OPTIONS   :=

.PHONY: help
.DEFAULT: help
.SILENT: help
help::
	echo "Usage: make <target>"
	echo

include Makefile.setup
include Makefile.all

# Workspace targets -----------------------------------------------------------
.PHONY: lockfile status deploy sphinx doxygen docker-all docker-test docker-release purge-all smoketest
help::
	echo "Available workspace targets:"
	echo "  smoketest       to run BATS system tests"
	echo "  sphinx          to generate Sphinx documentation"
	echo "  doxygen         to generate Doxygen documentation"
	echo "  deploy          to deploy Cloe to INSTALL_DIR [=${INSTALL_DIR}]"
	echo "  deploy-cli      to install cloe-launch with pip"
	echo
	echo "  docker-test     to build only a single Docker image"
	echo "  docker-all      to build all Docker images"
	echo "  docker-release  to upload all Conan packages from Docker images"
	echo

${BUILD_LOCKFILE}:
	${MAKE} -f Makefile.package LOCKFILE_SOURCE=${LOCKFILE_SOURCE} ${BUILD_LOCKFILE}

lockfile: ${BUILD_LOCKFILE}

status: ${BUILD_LOCKFILE}
	@for pkg in ${ALL_PKGS}; do \
		[ -d $${pkg} ] || continue; \
		${MAKE} LOCKFILE_SOURCE="" LOCKFILE_OPTION=${LOCKFILE_OPTION} -C $${pkg} status || true; \
	done

deploy:
	$(call print_header, "Deploying binaries to ${INSTALL_DIR}...")
	conan install ${CONAN_OPTIONS} --install-folder ${BUILD_DIR}/deploy -g deploy .
	mkdir -p ${INSTALL_DIR}
	cp -r ${BUILD_DIR}/deploy/cloe-*/* ${INSTALL_DIR}/

deploy-cli:
	$(call print_header, "Deploying cloe-launch binary with pip...")
	${MAKE} -C cli install

docker-test:
	$(call print_header, "Building ubuntu-18.04 Docker image...")
	${MAKE} -C dist/docker ubuntu-18.04

docker-all:
	$(call print_header, "Building all Docker images...")
	${MAKE} -C dist/docker all

docker-release:
	$(call print_header, "Uploading all Conan packages from Docker images...")
	${MAKE} -C dist/docker release

sphinx:
	$(call print_header, "Generating Sphinx documentation...")
	mkdir -p ${BUILD_DIR}/sphinx
	${MAKE} -C docs html

doxygen:
	$(call print_header, "Generating Doxygen documentation...")
	mkdir -p ${BUILD_DIR}/doxygen
	doxygen Doxyfile

smoketest:
	# Call this target with WITH_VTD=1 to include VTD binding tests.
	$(call print_header, "Running smoke tests...")
	@${CLOE_LAUNCH} clean -P conantest.py
	@\time -f "\nTotal smoketest time (real) = %e sec" bash -c "source tests/setup_testname.bash && bats tests"

purge-all:
	$(call print_header, "Removing all cloe Conan packages...")
	conan remove -f 'cloe-*'
	conan remove -f 'cloe'
	conan remove -f 'fable'

# Development targets ---------------------------------------------------------
.PHONY: format todos find-missing-eol sanitize-files
help::
	echo "Available development targets:"
	echo "  status          to show current workspace status"
	echo "  format         to format Cloe source code with clang-format"
	echo "  todos          to show all TODOs in Cloe source code"
	echo

format:
	find . -type f -not -path '*/\.git/*' -and \( -name '*.cpp' -o -name '*.hpp' \) -exec ${CLANG_FORMAT} ${CLANG_FORMAT_ARGS} -i {} \;

todos:
	${AG} TODO
	${AG} FIXME
	${AG} XXX

grep-uuids:
	${AG} "\b[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\b"

find-missing-eol:
	find . -type f -size +0 -exec gawk 'ENDFILE{if ($0 == "") print FILENAME}' {} \;

sanitize-files:
	git grep --cached -Ilz '' | while IFS= read -rd '' f; do tail -c1 < "$$f" | read -r _ || echo >> "$$f"; done
