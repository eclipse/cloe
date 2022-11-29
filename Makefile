# Makefile
#
# This file contains Makefile targets for the cloe project.
#

CLOE_ROOT   := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
CLOE_LAUNCH := PYTHONPATH="${CLOE_ROOT}/cli" python3 -m cloe_launch

include ${CLOE_ROOT}/Makefile.help

# Set the clang-format command line to use:
CLANG_FORMAT := $(shell command -v clang-format 2>/dev/null)
CLANG_FORMAT_ARGS := -style=file

AG := $(or \
	$(shell command -v rg 2>/dev/null), \
	$(shell command -v ag 2>/dev/null), \
	"grep -r" \
)

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
	$(call print_help_usage)
	echo

# Setup targets ---------------------------------------------------------------
include Makefile.setup

# Workspace targets -----------------------------------------------------------
.PHONY: lockfile status deploy sphinx doxygen purge-all export-cli smoketest smoketest-deps
help::
	$(call print_help_section, "Available workspace targets")
	$(call print_help_target, status, "show status of each of the Conan packages")
	$(call print_help_target, smoketest-deps, "build system test pre-requisites")
	$(call print_help_target, smoketest, "run system tests")
	$(call print_help_target, sphinx, "generate Sphinx documentation")
	$(call print_help_target, doxygen, "generate Doxygen documentation")
	$(call print_help_target, deploy, "deploy Cloe to INSTALL_DIR [=${INSTALL_DIR}]")
	$(call print_help_target, deploy-cli, "install ${_yel}cloe-launch${_rst} with ${_dim}${PIPX}${_rst}")
	$(call print_help_target, export-cli, "export ${_yel}cloe-launch-profile${_rst} Conan recipe")
	echo

${BUILD_LOCKFILE}:
	${MAKE} -f Makefile.package SOURCE_CONANFILE=/dev/null LOCKFILE_SOURCE=${LOCKFILE_SOURCE} ${BUILD_LOCKFILE}

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

export-cli:
	${MAKE} -C cli conan-profile

sphinx:
	$(call print_header, "Generating Sphinx documentation...")
	mkdir -p ${BUILD_DIR}/sphinx
	${MAKE} -C docs html

doxygen:keyboard tilt
	$(call print_header, "Generating Doxygen documentation...")
	mkdir -p ${BUILD_DIR}/doxygen
	doxygen Doxyfile

smoketest-deps: export-cli smoketest-deps-select

smoketest: smoketest-select

purge-all:
	$(call print_header, "Removing all cloe Conan packages...")
	conan remove -f 'cloe-*'
	conan remove -f 'cloe'
	conan remove -f 'fable'

# Development targets ---------------------------------------------------------
.PHONY: format todos find-missing-eol sanitize-files
help::
	$(call print_help_section, "Available development targets")
	$(call print_help_target, format, "format Cloe source code with clang-format")
	$(call print_help_target, todos, "show all TODOs in Cloe source code")
	echo

format:
	# When you do this, isolate the change in a single commit and then add the
	# commit hash to a file such as .git-blame-ignore-revs, so that git-blame
	# continues to work as expected.
	#
	# See: https://www.moxio.com/blog/43/ignoring-bulk-change-commits-with-git-blame
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

# Build targets ---------------------------------------------------------------
include Makefile.all
