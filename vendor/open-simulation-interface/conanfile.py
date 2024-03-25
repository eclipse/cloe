# mypy: ignore-errors
# pylint: skip-file

import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools import build, cmake, files

required_conan_version = ">=1.60.0 <2.0 || >=2.0.5"


class OpenSimulationInterfaceConan(ConanFile):
    name = "open-simulation-interface"
    description = "Generic interface environmental perception of automated driving functions in virtual scenarios"
    license = "MPL-2.0"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/OpenSimulationInterface/open-simulation-interface"
    topics = ("asam", "adas", "open-simulation", "automated-driving", "openx")
    package_type = "library"
    generators = "CMakeDeps", "CMakeToolchain"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def export_sources(self):
        files.export_conandata_patches(self)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake.cmake_layout(self)

    def requirements(self):
        self.requires("protobuf/3.21.12", transitive_headers=True, transitive_libs=True)

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            build.check_min_cppstd(self, 11)
        if self.options.shared and self.settings.os == "Windows":
            raise ConanInvalidConfiguration(
                "Shared Libraries are not supported on windows because of the missing symbol export in the library."
            )

    def build_requirements(self):
        self.tool_requires("protobuf/<host_version>")

    def source(self):
        files.get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def build(self):
        files.apply_conandata_patches(self)
        cm = cmake.CMake(self)
        if self.should_configure:
            cm.configure()
        if self.should_build:
            cm.build()

    def package(self):
        if not self.should_install:
            return
        cm = cmake.CMake(self)
        cm.install()
        files.copy(self, "LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        if self.settings.os == "Windows":
            files.rmdir(self, os.path.join(self.package_folder, "CMake"))
        else:
            files.rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "open_simulation_interface")
        self.cpp_info.set_property("cmake_target_name", "open_simulation_interface::open_simulation_interface")
        self.cpp_info.components["libopen_simulation_interface"].libs = ["open_simulation_interface"]
        self.cpp_info.components["libopen_simulation_interface"].requires = ["protobuf::libprotobuf"]

        # TODO: to remove in conan v2 once cmake_find_package_* generators removed
        self.cpp_info.names["cmake_find_package"] = "open_simulation_interface"
        self.cpp_info.names["cmake_find_package_multi"] = "open_simulation_interface"
        self.cpp_info.components["libopen_simulation_interface"].names["cmake_find_package"] = "open_simulation_interface"
        self.cpp_info.components["libopen_simulation_interface"].names["cmake_find_package_multi"] = "open_simulation_interface"
