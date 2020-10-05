include ../Makefile.package

# If BUILD_TESTS is disabled, we need to use the dependencies that
# having testing disabled.
ifeq (${BUILD_TESTS},0)
	CONAN_OPTIONS += -o test=False
	CONAN_OPTIONS += -o fable:test=False
	CONAN_OPTIONS += -o cloe-models:test=False
	CONAN_OPTIONS += -o cloe-runtime:test=False
endif
