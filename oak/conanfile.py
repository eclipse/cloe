from pathlib import Path
from conans import CMake, ConanFile, RunEnvironment, tools


class CloeOak(ConanFile):
    name = "cloe-oak"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe web-server backend implementation"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "test": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "test": True,
        "pedantic": True,
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "include/*",
        "src/*",
        "CMakeLists.txt",
    ]
    requires = ["boost/[>=1.66.0]"]

    _cmake = None

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            with tools.environment_append(RunEnvironment(self).vars):
                cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        if self.in_local_cache:
            self.cpp_info.libs = tools.collect_libs(self)
        else:
            self.cpp_info.libs = ["cloe-oak"]
