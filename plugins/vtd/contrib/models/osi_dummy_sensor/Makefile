OSI_PROTOBUF_VERSION := 2.6.1

OSI_LIB_NAME := libopen_simulation_interface.so
OSI_SEARCH_PATH := ${HOME}/VTD/Data
OSI_LIB_PATH := $(shell dirname $(shell find ${OSI_SEARCH_PATH} -name ${OSI_LIB_NAME} | head -1))

ifndef LINK_TRG_PATH
LINK_TRG_PATH := ${HOME}/VTD/Data/Distros/Distro/Plugins/ModuleManager
endif

CONAN_USER := cloe
CONAN_CHANNEL := vtd2p2 # e.g. stable, testing, ..
COMPILER := gcc
COMPILER_VERSION := 5
COMPILER_CPPSTD := 14
COMPILER_LIBCXX := libstdc++
ARCH := x86_64
OS := Linux
BUILD_TYPE := Release

PKG_NAME := $(shell grep "^\s*name\s*=.*\".*\"" conanfile.py | sed "s/.*=\s*\"//g" | sed "s/\"//g")
PKG_SETTINGS := \
		-s compiler=${COMPILER} \
		-s compiler.version=${COMPILER_VERSION} \
		-s compiler.cppstd=${COMPILER_CPPSTD} \
		-s compiler.libcxx=${COMPILER_LIBCXX} \
		-s arch=${ARCH} \
		-s os=${OS} \
		-s build_type=${BUILD_TYPE}

PKG_OPTIONS := \
		-o ${PKG_NAME}:link_osi_shared=True \
		-o ${PKG_NAME}:osi_lib_path=${OSI_LIB_PATH} \
		-o ${PKG_NAME}:link_trg_path=${LINK_TRG_PATH} \
		-o open-simulation-interface:link_protobuf_shared=True \
		-o open-simulation-interface:protobuf_version=${OSI_PROTOBUF_VERSION} \
		-o open-simulation-interface:protobuf_distr=${CONAN_USER}/${CONAN_CHANNEL} \
		-o open-simulation-interface:protoc_from_protobuf=True

.PHONY: all
all: dummy_sensor

.PHONY: dummy_sensor
dummy_sensor: conanfile.py found_osi protobuf
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

.PHONY: protobuf
protobuf:
	make -C vendor/protobuf protobuf clean_build clean_source

.PHONY: found_osi
found_osi:
ifeq (${OSI_LIB_PATH},)
	$(error ${OSI_LIB_NAME} not found in ${OSI_SEARCH_PATH}: ${OSI_LIB_PATH})
endif
