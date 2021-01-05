from pathlib import Path

from conans import CMake, ConanFile, tools
from conans.errors import ConanInvalidConfiguration


class VtdApiConan(ConanFile):
    name = "vtd-api"
    version = "2.2.0"
    license = "Proprietary"
    url = "https://vires.mscsoftware.com"
    no_copy_source = True
    description = "Include binary files in C/C++"
    topics = ("simulator", "vires")
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }
    generators = "cmake"

    _archive_base = "../vtd/vtd.2.2.0.Base.20181231.tgz"
    _root_dir = "VTD.2.2"

    def export_sources(self):
        tools.untargz(
            self._archive_base,
            self.export_sources_folder,
            pattern=Path(self._root_dir) / "Develop" / "*",
        )
        self.copy("CMakeLists.txt")

    def configure(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("VTD binaries do not exist for Windows")

    @property
    def cmake(self):
        if hasattr(self, "_cmake"):
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.configure()
        return self._cmake

    def build(self):
        self.cmake.build()

    def package(self):
        self.copy("Develop", src=self._root_dir, symlinks=True)
        self.cmake.install()

    def package_info(self):
        if self.in_local_cache:
            self.cpp_info.libs = tools.collect_libs(self)
        else:
            self.cpp_info.libs = ["vtd_api"]
