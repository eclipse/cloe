from pathlib import Path

from conans import CMake, ConanFile, tools
from conans.errors import ConanInvalidConfiguration


class VtdApiConan(ConanFile):
    name = "vtd-api"
    version = "2.2.0"
    license = "Proprietary"
    url = "https://vires.mscsoftware.com"
    no_copy_source = False
    description = "Include binary files in C/C++"
    topics = ("simulator", "vires")
    settings = "os", "compiler", "build_type", "arch"
    keep_imports = True
    exports_sources = [
        "CMakeLists.txt",
    ]
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }
    build_requires = [
        "vtd/2.2.0@cloe-restricted/stable",
    ]
    generators = "cmake"

    _cmake = None

    def configure(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("VTD binaries do not exist for Windows")

    def imports(self):
        self.copy("*", dst="src/Develop", src="Develop", root_package="vtd")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy("Develop", src="src", symlinks=True)

    def package_info(self):
        if self.in_local_cache:
            self.cpp_info.libs = tools.collect_libs(self)
        else:
            self.cpp_info.libs = ["vtd_api"]
