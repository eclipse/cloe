# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools import cmake, files

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
    build_requires = [
        "vtd/2.2.0@cloe-restricted/stable",
    ]
    generators = "CMakeDeps", "CMakeToolchain"

    def configure(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("VTD binaries do not exist for Windows")

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        files.copy(self, "Develop/*", src=self.dependencies.build["vtd"].package_folder, dst="../src")

    def build(self):
        cm = cmake.CMake(self)
        cm.configure()
        cm.build()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()
        files.copy(self, "Develop", src="src", dst=".")

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "vtd-api")
        self.cpp_info.set_property("cmake_target_name", "vtd::api")
        self.cpp_info.set_property("pkg_config_name", "vtd-api")

        # This define takes the format:
        #
        #   (EPOCH << 24) | (MAJOR_VERSION << 16) | (MINOR_VERSION << 8) | PATCH_VERSION
        #
        # Each version consists of at most 8 bits, so 256 potential values, including 0.
        # The epoch starts with 0, and is bumped after each version naming scheme.
        #
        # When years are involved, such as in releases versioned YYYY.MM, use the last
        # two years, since the full year will not fit in the 8 bits.
        #
        # VTD VERSION   : (0)       2        . 2     . 0
        vtd_api_version = (0<<24) | (2<<16) | (2<<8) | 0
        self.cpp_info.defines = [f"VTD_API_VERSION={vtd_api_version}"]

        if not self.in_local_cache:
            self.cpp_info.libs = ["vtd_api"]
        else:
            self.cpp_info.libs = files.collect_libs(self)
