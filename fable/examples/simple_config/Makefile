# This is a minimal Makefile to show how this example project
# can be built with Conan and CMake.

# All generated and compiled output is in this directory.
BUILD_DIR := build

# The build type must be synchronized between Conan (host build type)
# and CMake, otherwise you will get strange and unhelpful errors.
BUILD_TYPE := Release

# This is the output that Conan generates when using
# `CMakeToolchain` generator AND the `cmake_layout` layout.
TOOLCHAIN_FILE := ${BUILD_DIR}/generators/conan_toolchain.cmake

.PHONY: all clean
all: ${TOOLCHAIN_FILE}
	cmake --build ${BUILD_DIR}

${TOOLCHAIN_FILE}:
	conan install . --build=missing --install-folder=${BUILD_DIR} -s:h build_type=${BUILD_TYPE}
	cmake -B ${BUILD_DIR} -S . \
		-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
		-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1
	cmake --build ${BUILD_DIR}

clean:
	-rm -r ${BUILD_DIR}
