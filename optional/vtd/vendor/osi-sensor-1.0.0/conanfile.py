# mypy: ignore-errors
# pylint: skip-file

import os
import shutil
from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, files

required_conan_version = ">=1.52.0"


class VtdSensorConan(ConanFile):
    name = "osi-sensor"
    version = "1.0.0"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/OpenSimulationInterface/osi-sensor-model-packaging"
    description = "Example of a sensor model using OSI with FMI 2.0."
    topics = ("Sensor Simulation", "HAD")
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "VirtualRunEnv"
    options = {
        "simulator": ["vtd-2.2.0"],
    }
    default_options = {
        "simulator": "vtd-2.2.0",
    }

    def export_sources(self):
        files.export_conandata_patches(self)

    def requirements(self):
        self.requires("open-simulation-interface/3.0.1@cloe/stable", private=True)
        self.requires(
            "protobuf/2.6.1@cloe/stable",
            transitive_headers=True,
            transitive_libs=True,
            private=True,
        )
        self.requires("vtd/2.2.0@cloe-restricted/stable")

    def source(self):
        files.get(
            self,
            **self.conan_data["sources"][self.version],
            destination=".",
            strip_root=True,
        )

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
        tc.cache_variables["VTD_ROOT"] = self.deps_env_info["vtd"].VTD_ROOT
        tc.generate()

    def build(self):
        vtd_root = Path(self.deps_env_info["vtd"].VTD_ROOT)
        osi_lib = (
            vtd_root / "/Data/Setups/Standard.OSI3/Bin/libopen_simulation_interface.so"
        )
        if not osi_lib.is_file():
            self.output.warn(f"VTD OSI library not found: {osi_lib}")

        cm = cmake.CMake(self)
        if self.should_configure:
            files.apply_conandata_patches(self)
            cm.configure(
                build_script_folder=os.path.join(
                    self.source_folder, "examples", "OSMPDummySensor"
                )
            )
        if self.should_build:
            cm.build()

    def package(self):
        cm = cmake.CMake(self)
        if self.should_install:
            cm.install()

    def package_info(self):
        self.runenv_info.append(
            "VTD_EXTERNAL_MODELS",
            os.path.join(self.package_folder, "lib", "OSMPDummySensor.so"),
        )
