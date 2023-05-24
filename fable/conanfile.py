# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path
from semver import SemVer

from conan import ConanFile
from conan.tools import cmake, files, scm

required_conan_version = ">=1.52.0"


class Fable(ConanFile):
    name = "fable"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "JSON schema and configuration library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "allow_comments": [True, False],
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "allow_comments": True,
        "shared": False,
        "fPIC": True,
    }
    generators = "CMakeDeps", "VirtualRunEnv"
    no_copy_source = True
    exports_sources = [
        "cmake/*",
        "include/*",
        "examples/*",
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
        self.requires("fmt/9.1.0")
        self.requires("nlohmann_json/3.11.2")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")
        self.test_requires("boost/[>=1.65.1]")

    def layout(self):
        cmake.cmake_layout(self)
        self.cpp.source.includedirs.append(os.path.join(self.folders.build, "include"))

    def generate(self):
        # The version as a single 32-bit number takes the format:
        #
        #   (EPOCH << 24) | (MAJOR_VERSION << 16) | (MINOR_VERSION << 8) | PATCH_VERSION
        #
        # Each version consists of at most 8 bits, so 256 potential values, including 0.
        # The epoch starts with 0, and is bumped after each version naming scheme.
        semver = SemVer(self.version, True)
        version_u32 = (0<<24) | (semver.major << 16) | (semver.minor << 8) | semver.patch

        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["FABLE_VERSION"] = self.version
        tc.cache_variables["FABLE_VERSION_U32"] = version_u32
        tc.cache_variables["FABLE_ALLOW_COMMENTS"] = self.options.allow_comments
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        if self.should_configure:
            cm.configure()
        if self.should_build:
            cm.build()
        if self.should_test:
            cm.test()

    def package(self):
        cm = cmake.CMake(self)
        if self.should_install:
            cm.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "fable")
        self.cpp_info.set_property("cmake_target_name", "fable::fable")
        self.cpp_info.set_property("pkg_config_name", "fable")
        if not self.in_local_cache:
            self.cpp_info.libs = ["fable"]
            self.cpp_info.includedirs.append(os.path.join(self.build_folder, "include"))
        else:
            self.cpp_info.libs = files.collect_libs(self)
