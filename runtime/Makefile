PROJECT_ROOT := ..

override BUILD_LAYOUT := layout.ini

include ${PROJECT_ROOT}/Makefile.package

# If BUILD_TESTS is disabled, we need to use the dependencies that
# having testing disabled.
ifeq (${BUILD_TESTS},0)
	CONAN_OPTIONS += -o test=False
	CONAN_OPTIONS += -o fable:test=False
endif
