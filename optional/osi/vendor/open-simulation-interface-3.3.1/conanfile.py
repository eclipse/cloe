# mypy: ignore-errors
# pylint: skip-file

import os
import shutil

from conans import CMake, ConanFile, tools


class OpenSimulationInterfaceConan(ConanFile):
    name = "open-simulation-interface"
    version = "3.3.1"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/OpenSimulationInterface/open-simulation-interface"
    description = "A generic interface for the environmental perception of automated driving functions in virtual scenarios."
    topics = ("Sensor Simulation", "HAD")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
    }
    generators = "cmake"
    build_policy = "missing"
    no_copy_source = False
    exports_sources = [
        "CMakeLists.txt",
    ]
    requires = [
        "protobuf/[~=3.15.2]",
    ]

    _git_url = (
        "https://github.com/OpenSimulationInterface/open-simulation-interface.git"
    )
    _git_dir = "osi"
    _git_ref = f"v{version}"

    _cmake = None

    def configure(self):
        self.options["protobuf"].shared = self.options.shared

    def source(self):
        git = tools.Git(folder=self._git_dir)
        git.clone(self._git_url, self._git_ref, shallow=True)
        dst = os.path.join(self.source_folder, self._git_dir)
        shutil.copy("CMakeLists.txt", dst)

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_PROJECT_VERSION"] = self.version
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.configure(source_folder=self._git_dir)
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_id(self):
        self.info.requires["protobuf"].full_package_mode()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
