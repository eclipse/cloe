# This is a minimal Makefile to show how this example project
# can be built with Conan and CMake.

# All generated and compiled output is in this directory.
BUILD_DIR := build

# The build type must be synchronized between Conan (host build type)
# and CMake, otherwise you will get strange and unhelpful errors.
BUILD_TYPE := Release
CMAKE_BUILD_TYPE := $(shell echo ${BUILD_TYPE} | tr '[:upper:]' '[:lower:]')

# This is the output that Conan generates when using
# `CMakeToolchain` generator AND the `cmake_layout` layout.
TOOLCHAIN_FILE := ${BUILD_DIR}/${BUILD_TYPE}/generators/conan_toolchain.cmake

# How many variables should the "Large" struct have?
LARGE_STRUCT_SIZE := 1000

FABLE_VERSION := $(shell make --quiet -C ../.. info-fqn)

.PHONY: all clean
all: ${TOOLCHAIN_FILE}
	cmake --build --preset=${CMAKE_BUILD_TYPE}

${TOOLCHAIN_FILE}:
	conan install . --build=missing --install-folder=${BUILD_DIR} -s:h build_type=${BUILD_TYPE} --require-override ${FABLE_VERSION}
	cmake --preset=${CMAKE_BUILD_TYPE} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DLARGE_STRUCT_SIZE=${LARGE_STRUCT_SIZE}

clean:
	-rm -r ${BUILD_DIR}
	-rm CMakeUserPresets.json
