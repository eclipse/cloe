from pathlib import Path
from conans import CMake, ConanFile, RunEnvironment, tools


class Fable(ConanFile):
    name = "fable"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "JSON schema and configuration library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "test": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "test": True,
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "include/*",
        "examples/*",
        "src/*",
        "CMakeLists.txt",
    ]
    requires = [
        "boost/[>=1.65.1]",
        "fmt/[>=6.2.0]",
        "nlohmann_json/[~=3.10.5]",
    ]

    _cmake = None

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~=1.10]")
            self.build_requires("cli11/[~=2.1.2]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["FABLE_VERSION"] = self.version
        self._cmake.definitions["BuildTests"] = self.options.test
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

    def package_id(self):
        self.info.requires["boost"].full_package_mode()

    def package_info(self):
        if self.in_local_cache:
            self.cpp_info.libs = tools.collect_libs(self)
        else:
            self.cpp_info.libs = ["fable"]
