# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, files, scm

required_conan_version = ">=1.52.0"

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
    generators = "CMakeDeps", "VirtualRunEnv"
    no_copy_source = True
    exports_sources = [
        "cmake/*",
        "include/*",
        "src/*",
        "CMakeLists.txt",
    ]

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"fable/{self.version}@cloe/develop")
        self.requires("boost/[>=1.65.1]")
        self.requires("inja/3.4.0")
        self.requires("spdlog/1.11.0")
        self.requires("incbin/cci.20211107")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CLOE_PROJECT_VERSION"] = self.version
        tc.cache_variables["TargetLintingExtended"] = self.options.pedantic
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        cm.configure()
        cm.build()
        cm.test()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        del self.info.options.pedantic

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe-runtime")
        self.cpp_info.set_property("cmake_target_name", "cloe::runtime")
        self.cpp_info.set_property("pkg_config_name", "cloe-runtime")

        # Make sure we can find the libs and *.cmake files, both in editable
        # mode and in the normal package mode:
        # TODO: Is this still necessarytools?
        if not self.in_local_cache:
            self.cpp_info.builddirs.append("cmake")
            self.cpp_info.libs = ["cloe-runtime"]
        else:
            self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "cloe"))
            self.cpp_info.libs = files.collect_libs(self)
