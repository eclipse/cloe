# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path

from conans import CMake, ConanFile, RunEnvironment, tools


class CloeRuntime(ConanFile):
    name = "cloe-runtime"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe runtime"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "pedantic": True,
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "cmake/*",
        "include/*",
        "src/*",
        "CMakeLists.txt",
    ]
    requires = [
        "boost/[>=1.65.1]",
        "inja/[~=3.3.0]",
        "spdlog/[~=1.9.0]",
    ]

    _cmake = None

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"fable/{self.version}@cloe/develop")
        self.requires("incbin/cci.20211107")

    def build_requirements(self):
        self.test_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.definitions["CLOE_PROJECT_VERSION"] = self.version
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        with tools.environment_append(RunEnvironment(self).vars):
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        del self.info.options.pedantic

    def package_info(self):
        # Make sure we can find the libs and *.cmake files, both in editable
        # mode and in the normal package mode:
        if not self.in_local_cache:
            self.cpp_info.builddirs.append("cmake")
            self.cpp_info.libs = ["cloe-runtime"]
        else:
            self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "cloe"))
            self.cpp_info.libs = tools.collect_libs(self)
