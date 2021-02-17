VTD_OSI_VERSION := 3.0.1
VTD_OSI_CHANNEL := cloe/vtd2p2

# Overrides will be preserved in Makefile.package.
override SOURCE_DIR := src
override PACKAGE_VERSION := $(shell cat VERSION)
override PACKAGE_CHANNEL := cloe/stable
override CLEAN_SOURCE_DIR := true

include ../../Makefile.package

export:
	@(echo "${VTD_OSI_VERSION}" > VERSION)
	conan export . ${PACKAGE_NAME}/${VTD_OSI_VERSION}@${VTD_OSI_CHANNEL}
	@(echo "${PACKAGE_VERSION}" > VERSION)
	conan export . ${PACKAGE_NAME}/${PACKAGE_VERSION}@${PACKAGE_CHANNEL}
