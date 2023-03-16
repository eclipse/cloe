# mypy: ignore-errors
# pylint: skip-file

import os
import shutil
from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, scm, files

required_conan_version = ">=1.52.0"


class VtdSensorConan(ConanFile):
    name = "osi-sensor"
    version = "1.0.0-vtd2.2"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/OpenSimulationInterface/osi-sensor-model-packaging"
    description = "Example of a sensor model using OSI with FMI 2.0."
    topics = ("Sensor Simulation", "HAD")
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    build_policy = "missing"
    _patch_vtd2p2 = "patches/osmp_vtd_v2.2.patch"  # git format-patch v1.0.0 --stdout > osmp_vtd_v2.2.patch
    exports_sources = [
        _patch_vtd2p2,
        "CMakeLists.txt",
    ]
    _git_url = (
        "https://github.com/OpenSimulationInterface/osi-sensor-model-packaging.git"
    )
    _git_ref = "v{}".format(version.split("-")[0])
    _git_dir = "osmp"
    _rel_src_dir = os.path.join(_git_dir, "examples/OSMPDummySensor")
    _pkg_lib = "OSMPDummySensor.so"

    def requirements(self):
        self.requires("open-simulation-interface/[=3.0.*]@cloe/stable", private=True)
        self.requires("protobuf/2.6.1@cloe/stable", override=True)
        self.requires("vtd/2.2.0@cloe-restricted/stable")

    def source(self):
        git = scm.Git(self, self._git_dir)
        git.clone(self._git_url, args=["--depth=1", "--branch", self._git_ref])
        dst = os.path.join(self.source_folder, self._rel_src_dir)
        shutil.copy("CMakeLists.txt", dst)

    def configure(self):
        # requirement options
        self.options["open-simulation-interface"].shared = True
        # package options
        self.settings.compiler.cppstd = "11"
        self.settings.compiler.libcxx = "libstdc++"

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_PROJECT_VERSION"] = self.version
        tc.cache_variables["CMAKE_BUILD_TYPE"] = self.settings.get_safe("build_type")
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.generate()

    def build(self):
        # Find VTD osi library
        osi_lib_dir = Path(
            self.deps_env_info["vtd"].VTD_ROOT + "/Data/Setups/Standard.OSI3/Bin/"
        )
        if not Path(str(osi_lib_dir) + "/libopen_simulation_interface.so").is_file():
            self.output.warn(f"VTD OSI library not found: {osi_lib_dir}")

        # Apply patch for compatibility with VTD osi plugin
        base_path = self._git_dir
        patch_file = self._patch_vtd2p2
        if not self.in_local_cache:
            base_path = self.source_folder + "/" + base_path
            patch_file = self.recipe_folder + "/" + patch_file
        files.patch(self, base_path=base_path, patch_file=patch_file)

        # Configure and build
        cm = cmake.CMake(self)
        cm.configure(build_script_folder=self._rel_src_dir)
        cm.build()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "osi-sensor")
        self.cpp_info.set_property("cmake_target_name", "osi-sensor::osi-sensor")
        self.cpp_info.set_property("pkg_config_name", "osi-sensor")

        self.cpp_info.libs = files.collect_libs(self)
        pkg_path = os.path.join(self.package_folder, "lib", self._pkg_lib)
        # Collect the library path for linking the model to the VTD runtime setup
        self.env_info.VTD_EXTERNAL_MODELS.append(pkg_path)
