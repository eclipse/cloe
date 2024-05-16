# Makefile
#
# This file contains Makefile targets for the cloe project.
#

# Make configuration:
SHELL := /bin/bash
GNUMAKEFLAGS := --no-print-directory
SUBMAKEFLAGS :=

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
INSTALL_DIR     := /usr/local
DEPLOY_DIR      := deploy
CONAN_OPTIONS   :=

# Lockfile for cloe-deployment:
DEPLOY_LOCKFILE_SOURCE := tests/conanfile_deployment.py
DEPLOY_BUILD_LOCKFILE  := ${DEPLOY_DIR}/conan.lock
DEPLOY_LOCKFILE_OPTION := --lockfile="${CLOE_ROOT}/${DEPLOY_BUILD_LOCKFILE}"

.DEFAULT_GOAL := help
.PHONY: help
.SILENT: help
help::
	$(call print_help_usage)
	echo
	$(call print_help_section, "Default target")
	$(call print_help_target, help, "show this help on available targets")
	echo

# Setup targets ---------------------------------------------------------------
include Makefile.setup

${DEPLOY_BUILD_LOCKFILE}:
	mkdir -p "${DEPLOY_DIR}"
	conan lock create --lockfile-out "${DEPLOY_BUILD_LOCKFILE}" --build -- "${DEPLOY_LOCKFILE_SOURCE}"

.PHONY: lockfile
lockfile: ${DEPLOY_BUILD_LOCKFILE}

# Workspace targets -----------------------------------------------------------
help::
	$(call print_help_section, "Available workspace targets")
	$(call print_help_target, docs, "generate Doxygen and Sphinx documentation")
	echo

.PHONY: docs
docs:
	$(call print_header, "Generating Doxygen documentation...")
	${MAKE} -C docs doxygen
	$(call print_header, "Generating Sphinx documentation...")
	${MAKE} -C docs html

help::
	$(call print_help_target, export-cli, "export ${_yel}cloe-launch-profile${_rst} Conan recipe")
	$(call print_help_target, deploy-cli, "install ${_yel}cloe-launch${_rst} with ${_dim}${PIPX}${_rst}")
	echo

.PHONY: export-cli
export-cli:
	${MAKE} -C cli export

.PHONY: deploy-cli
deploy-cli:
	$(call print_header, "Deploying cloe-launch binary with pip...")
	${MAKE} -C cli install

help::
	$(call print_help_target, lockfile, "create a lockfile for cloe deployment packages")
	$(call print_help_target, package-all, "package all cloe deployment packages")
	$(call print_help_target, status-all, "show status of each of the Conan packages")
	$(call print_help_target, export-all, "export all package sources to Conan cache")
	$(call print_help_target, build-all, "build individual packages locally in-source")
	$(call print_help_target, deploy-all, "deploy Cloe to INSTALL_DIR [=${INSTALL_DIR}]")
	$(call print_help_target, clean-all, "clean entire repository of temporary files")
	$(call print_help_target, purge-all, "remove all cloe packages (in any version) from Conan cache")
	echo

.PHONY: build-all
build-all: lockfile
	${MAKE} all-select CONAN_OPTIONS="${CONAN_OPTIONS} ${DEPLOY_LOCKFILE_OPTION}"

.PHONY: status-all
status-all: ${DEPLOY_BUILD_LOCKFILE}
	@for pkg in ${ALL_PKGS}; do \
		${MAKE} LOCKFILE_SOURCE="" LOCKFILE_OPTION=${DEPLOY_LOCKFILE_OPTION} -C $${pkg} status || true; \
	done

.PHONY: export-all
export-all:
	$(call print_header, "Exporting all cloe Conan packages...")
	${MAKE} export-select export-cli export

.PHONY: deploy-all
deploy-all:
	$(call print_header, "Deploying binaries to ${INSTALL_DIR}...")
	conan install ${CONAN_OPTIONS} --install-folder ${DEPLOY_DIR} -g deploy .
	mkdir -p ${INSTALL_DIR}
	cp -r ${DEPLOY_DIR}/cloe-*/* ${INSTALL_DIR}/

.PHONY: clean-all
clean-all:
	${MAKE} clean clean-select

.PHONY: purge-all
purge-all:
	$(call print_header, "Removing all cloe Conan packages...")
	conan remove -f 'cloe-*'
	conan remove -f 'cloe'
	conan remove -f 'fable'

.PHONY: package-all
package-all:
	conan install ${CONAN_OPTIONS} --install-folder ${DEPLOY_DIR} --build=missing --build=outdated ${DEPLOY_LOCKFILE_SOURCE}

# Development targets ---------------------------------------------------------
help::
	$(call print_help_section, "Available development targets")
	$(call print_help_target, format, "format Cloe source code with clang-format")
	$(call print_help_target, todos, "show all TODOs in Cloe source code")
	echo

.PHONY: format
format:
	# When you do this, isolate the change in a single commit and then add the
	# commit hash to a file such as .git-blame-ignore-revs, so that git-blame
	# continues to work as expected.
	#
	# See: https://www.moxio.com/blog/43/ignoring-bulk-change-commits-with-git-blame
	find . -type f -not -path '*/\.git/*' -and \( -name '*.cpp' -o -name '*.hpp' \) -exec ${CLANG_FORMAT} ${CLANG_FORMAT_ARGS} -i {} \;

.PHONY: todos
todos:
	${AG} TODO
	${AG} FIXME
	${AG} XXX

# Hidden development targets --------------------------------------------------

.PHONY: grep-uuids
grep-uuids:
	${AG} "\b[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\b"

grep-conan-requires:
	@rg -t py '^.*requires\(f?["](.+/[0-9]+\.[^)]+)["].*\).*$$' -r '$$1' -I --no-heading --no-line-number | sort | uniq

.PHONY: find-missing-eol
find-missing-eol:
	find . -type f -size +0 -exec gawk 'ENDFILE{if ($0 == "") print FILENAME}' {} \;

.PHONY: sanitize-files
sanitize-files:
	git grep --cached -Ilz '' | while IFS= read -rd '' f; do tail -c1 < "$$f" | read -r _ || echo >> "$$f"; done

# Micro-packages build targets ------------------------------------------------
include Makefile.all

# Mono-package build targets --------------------------------------------------
DISABLE_HELP_PREAMBLE := true
help::
	@printf "Available $(_yel)cloe$(_rst) package targets:\n"

include Makefile.package
