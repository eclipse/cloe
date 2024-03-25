import os

from conan import ConanFile
from conan.tools import build, cmake


class TestPackageConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "VirtualRunEnv"
    test_type = "explicit"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        cm.configure()
        cm.build()

    def test(self):
        if build.can_run(self):
            bin_path = os.path.join(self.build_folder, self.cpp.build.bindirs[0], "test_package")
            self.run(f"{bin_path} test-driver.xosc", env="conanrun", cwd=self.source_folder)
