CONAN_USER := cloe
CONAN_CHANNEL := vtd2p2
COMPILER := gcc
COMPILER_VERSION := 5
COMPILER_CPPSTD := 14
COMPILER_LIBCXX := libstdc++
ARCH := x86_64
OS := Linux
BUILD_TYPE := Release

PKG_NAME := $(shell grep "^\s*name\s*=.*\".*\"" conanfile.py | sed "s/.*=\s*\"//g" | sed "s/\"//g")
PKG_VERSION := $(shell grep "^\s*version\s*=.*\".*\"" conanfile.py | sed "s/.*=\s*\"//g" | sed "s/\"//g")

PKG_SETTINGS := \
		-s compiler=${COMPILER} \
		-s compiler.version=${COMPILER_VERSION} \
		-s compiler.cppstd=${COMPILER_CPPSTD} \
		-s compiler.libcxx=${COMPILER_LIBCXX} \
		-s arch=${ARCH} \
		-s os=${OS} \
		-s build_type=${BUILD_TYPE} \

.PHONY: all
all: protobuf

.PHONY: protobuf
protobuf: conanfile.py
	conan create . ${CONAN_USER}/${CONAN_CHANNEL} \
					${PKG_SETTINGS} \
					${PKG_OPTIONS} \
					--build missing

.PHONY: clean
clean: clean_build clean_source

.PHONY: clean_build
clean_build:
	conan remove "${PKG_NAME}/${PKG_VERSION}@${CONAN_USER}/${CONAN_CHANNEL}" -f -b

.PHONY: clean_source
clean_source:
	conan remove "${PKG_NAME}/${PKG_VERSION}@${CONAN_USER}/${CONAN_CHANNEL}" -f -s
