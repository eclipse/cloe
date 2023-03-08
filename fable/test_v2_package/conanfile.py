# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, build, files, scm

required_conan_version = ">=1.52.0"

class FableTestConan(ConanFile):
    name = "fable-examples"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    apply_env = False
    test_type = "explicit"

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def export_sources(self):
        files.copy(self, "CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        files.copy(self, "*", os.path.join(self.recipe_folder, "../examples/"), self.export_sources_folder)

    def requirements(self):
        if self.tested_reference_str:
            print(f"Using test reference: {self.tested_reference_str}")
            self.requires(self.tested_reference_str)
        else:
            self.requires(f"fable/{self.version}@cloe/develop")
        self.requires("cli11/2.3.2")
        self.requires("fmt/9.1.0")

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        if self.tested_reference_str:
            tc.variables["AS_TEST_PACKAGE"] = True
        tc.generate()

    def build(self):
        m = cmake.CMake(self)
        m.configure()
        m.build()

    def layout(self):
        cmake.cmake_layout(self)

    def test(self):
        if build.can_run(self):
            # contacts:
            testfile = os.path.join(self.recipe_folder, "../examples/contacts/example_addressbook.json")
            cmd = os.path.join(self.cpp.build.bindirs[0], "contacts", "contacts") + " -f " + testfile
            self.run(cmd, env="conanrun", output=False)

            # simple-config:
            testfile = os.path.join(self.recipe_folder, "../examples/simple_config/example_input.json")
            cmd = os.path.join(self.cpp.build.bindirs[0], "simple_config", "simple-config") + " -f " + testfile
            self.run(cmd, env="conanrun", output=False)
