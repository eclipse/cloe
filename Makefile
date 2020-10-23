# Makefile
# This file contains Makefile targets for the cloe project.
#

# Make configuration:
SHELL := /bin/bash
GNUMAKEFLAGS := --no-print-directory

# Set the clang-format command line to use:
CLANG_FORMAT := $(shell command -v clang-format 2>/dev/null)
CLANG_FORMAT_ARGS := -style=file

AG := $(or \
	$(shell command -v rg 2>/dev/null), \
	$(shell command -v ag 2>/dev/null), \
	"grep -r" \
)

CLOE_ROOT   := .
CLOE_LAUNCH := PYTHONPATH="${CLOE_ROOT}/cli" python3 -m cloe_launch

# Build configuration:
SOURCE_DIR   := .
BUILD_DIR    := build
INSTALL_DIR  := /usr/local

# These packages can all be built in parallel:
PACKAGES := engine $(wildcard plugins/*)

# This variable contains packages that should not be built.
# This should only be provided from the command line, for example:
#     make NOBUILD_PKGS="plugins/vtd plugins/demo_printer" package
NOBUILD_PKGS :=

# These options are passed to conan when it is working with the cloe metapackage.
CONAN_OPTIONS :=

# Options:
BUILD_TESTS := 1
ifeq (${BUILD_TESTS},0)
	CONAN_OPTIONS += -o cloe:test=False
	CONAN_OPTIONS += -o fable:test=False
	CONAN_OPTIONS += -o cloe-runtime:test=False
	CONAN_OPTIONS += -o cloe-models:test=False
	CONAN_OPTIONS += -o cloe-oak:test=False
	ifeq (${WITH_ENGINE},1)
		CONAN_OPTIONS += -o cloe-engine:test=False
	endif
	ifeq (${WITH_VTD},1)
		CONAN_OPTIONS += -o cloe-plugin-vtd:test=False
	endif
	CONAN_OPTIONS += -o cloe-plugin-basic:test=False
endif

WITH_ENGINE := 1
ifeq (${WITH_ENGINE},0)
	PACKAGES := $(filter-out engine, ${PACKAGES})
	CONAN_OPTIONS += -o cloe:with_engine=False
endif

WITH_VTD := 1
ifeq (${WITH_VTD},0)
	PACKAGES := $(filter-out plugins/vtd, ${PACKAGES})
	CONAN_OPTIONS += -o cloe:with_vtd=False
endif

# Take NOBUILD_PKGS into account to assemble a final list of
# packages that we shall build (in addition to fable, runtime, and models).
# This can also be provided from the command line.
BUILD_PKGS := $(filter-out ${NOBUILD_PKGS}, ${PACKAGES})

.PHONY: help
.DEFAULT: help
help::
	@echo "Usage: make <target>"
	@echo

# Setup Targets ----------------------------------------------------------------
include Makefile.setup

# Build Targets -----------------------------------------------------------------------------
help::
	@echo "Available workspace build targets:"
	@echo "  smoketest      to run BATS system tests"
	@echo "  clean          to clean all generated artifacts"
	@echo "  git-clean      to clean all extraneous artifacts from repository"
	@echo "  doc            to generate Doxygen documentation"
	@echo
	@echo "Available Conan build targets:"
	@echo "  export         to export all recipes to Conan cache"
	@echo "  package        to build and package all Cloe Conan packages"
	@echo "  purge          to remove all Cloe Conan packages from cache"
	@echo "  deploy         to install all Cloe Conan packages locally"
	@echo "  docker         to build all Docker images"
	@echo
	@echo "Available development targets:"
	@echo "  format         to format Cloe source code with clang-format"
	@echo "  todos          to show all TODOs in Cloe source code"
	@echo
	@echo "Options:"
	@echo "  BUILD_TESTS=(0|1)   to build and run unit tests"
	@echo "  WITH_ENGINE=(0|1)   to build and deploy cloe-engine (default=1)"
	@echo "  WITH_VTD=(0|1)      to build and deploy cloe-plugin-vtd (default=1)"
	@echo
	@echo "Defines:"
	@echo "  CONAN_OPTIONS = ${CONAN_OPTIONS}"
	@echo "  NOBUILD_PKGS = ${NOBUILD_PKGS}"
	@echo "  SOURCE_DIR = ${SOURCE_DIR}"
	@echo "  BUILD_DIR = ${BUILD_DIR}"
	@echo

.PHONY: clean clean-all test test-all package package-all deploy purge purge-all export smoketest git-clean docker

${BUILD_PKGS}::
	${MAKE} -C $@ $(shell echo ${MAKECMDGOALS} | sed 's/-all//')

export:
	conan export vendor/cpp-netlib cloe/stable
	conan export vendor/incbin cloe/stable
	conan export vendor/open-simulation-interface cloe/stable

smoketest:
	@echo ___________________________________________________________________________________
	@echo "Running smoketests..."
	@${CLOE_LAUNCH} clean -P conanfile.py
	@\time -f "\nTotal smoketest time (real) = %e sec" bats tests

doc:
	@echo ___________________________________________________________________________________
	@echo "Generating Doxygen documentation..."
	mkdir -p build/doxygen
	doxygen Doxyfile

docker:
	${MAKE} -C dist/docker all

docker-test:
	${MAKE} -C dist/docker ubuntu-18.04

clean-all: ${BUILD_PKGS}

clean:
	@echo ___________________________________________________________________________________
	@echo "Cleaning build files..."
	rm -rf ${BUILD_DIR}
	rm -f compile_commands.json
	${MAKE} -C runtime clean
	${MAKE} -C models clean
	${MAKE} clean-all

git-clean:
	-git clean -xdf -e .gtm

purge-all: ${BUILD_PKGS}

purge:
	@echo ___________________________________________________________________________________
	@echo "Removing Conan packages..."
	${MAKE} -C fable purge
	${MAKE} -C oak purge
	${MAKE} -C runtime purge
	${MAKE} -C models purge
	${MAKE} purge-all

package-all: ${BUILD_PKGS}

package:
	@echo ___________________________________________________________________________________
	@echo "Creating Conan packages..."
	conan export fable cloe/develop
	conan export oak cloe/develop
	${MAKE} -C runtime package
	${MAKE} -C models package
	${MAKE} package-all

package-outdated-all: ${BUILD_PKGS}

package-outdated:
	@echo ___________________________________________________________________________________
	@echo "Creating outdated Conan packages..."
	${MAKE} -C runtime package-outdated
	${MAKE} -C models package-outdated
	${MAKE} package-outdated-all

test-all: ${BUILD_PKGS}

test:
	@echo ___________________________________________________________________________________
	@echo "Running CMake tests..."
	${MAKE} -C runtime test
	${MAKE} -C models test
	${MAKE} test-all

deploy:
	@echo ___________________________________________________________________________________
	@echo "Deploying binaries to ${INSTALL_DIR}..."
	conan install ${CONAN_OPTIONS} --install-folder ${BUILD_DIR}/deploy -g deploy .
	mkdir -p ${INSTALL_DIR}
	cp -r ${BUILD_DIR}/deploy/cloe-*/* ${INSTALL_DIR}/

# Development targets -----------------------------------------------------------------------
.PHONY: todos format find-missing-eol sanitize-files
todos:
	${AG} TODO
	${AG} FIXME
	${AG} XXX

format:
	find . -type f -not -path '*/\.git/*' -and \( -name '*.cpp' -o -name '*.hpp' \) -exec ${CLANG_FORMAT} ${CLANG_FORMAT_ARGS} -i {} \;

sanitize-files:
	git grep --cached -Ilz '' | while IFS= read -rd '' f; do tail -c1 < "$$f" | read -r _ || echo >> "$$f"; done

find-missing-eol:
	find . -type f -size +0 -exec gawk 'ENDFILE{if ($0 == "") print FILENAME}' {} \;
