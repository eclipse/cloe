# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path
from conan import ConanFile
from conan.tools import cmake, files, scm

class CloeSimulatorESMini(ConanFile):
    name = "cloe-plugin-esmini"
    url = "https://github.com/esmini/esmini"
    description = "Cloe ESMini simulator plugin."
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "VirtualRunEnv"
    exports_sources = [
        "src/*",
        "CMakeLists.txt",
    ]
    no_copy_source = True

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires("eigen/3.4.0")
        self.requires(f"esmini/2.37.4@cloe/stable")
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")
        self.requires(f"cloe-osi/{self.version}@cloe/develop")

        self.requires("zlib/1.2.13", override=True)  # conflict between boost & protobuf

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CLOE_PROJECT_VERSION"] = self.version
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
        self.cpp_info.set_property("cmake_file_name", self.name)
        self.cpp_info.set_property("pkg_config_name", self.name)

        if not self.in_local_cache: # editable mode
            libdir = Path(self.build_folder) /  "lib"
            self.runenv_info.append_path("LD_LIBRARY_PATH", libdir)
