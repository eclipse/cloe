# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, files
from conan.errors import ConanInvalidConfiguration

required_conan_version = ">=1.52.0"


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
    generators = "CMakeDeps"
    requires = [
        ("vtd/2.2.0@cloe-restricted/stable", "private"),
    ]

    def configure(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("VTD binaries do not exist for Windows")

    def imports(self):
        self.copy("*", dst="src/Develop", src="Develop", root_package="vtd")

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        cm.configure()
        cm.build()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()
        self.copy("Develop", src="src", symlinks=True)

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "vtd-api")
        self.cpp_info.set_property("cmake_target_name", "vtd::api")
        self.cpp_info.set_property("pkg_config_name", "vtd-api")
        if self.in_local_cache:
            self.cpp_info.libs = files.collect_libs(self)
        else:
            self.cpp_info.libs = ["vtd_api"]
