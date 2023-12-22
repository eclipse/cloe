# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path
from conan import ConanFile
from conan.tools import cmake, files, scm


class CloeOsi(ConanFile):
    name = "cloe-osi"
    url = "https://github.com/eclipse/cloe"
    description = "OSI sensor component and utility functions for data conversion to Cloe"
    license = "Apache-2.0"
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
        "include/*",
        "src/*",
        "CMakeLists.txt",
    ]

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-models/{self.version}@cloe/develop")
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires("open-simulation-interface/3.3.1@cloe/stable")
        self.requires("boost/[>=1.65.1]")
        self.requires("eigen/3.4.0")

    def configure(self):
        self.options["open-simulation-interface"].shared = self.options.shared
        self.options["open-simulation-interface"].fPIC = True

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

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        self.info.requires["open-simulation-interface"].full_package_mode()
        del self.info.options.pedantic

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe-osi")
        self.cpp_info.set_property("cmake_target_name", "cloe::osi")
        self.cpp_info.set_property("pkg_config_name", "cloe-osi")

        # Make sure we can find the library, both in editable mode and in the
        # normal package mode:
        self.cpp_info.libdirs = [f"lib/osi"]
        if not self.in_local_cache:
            self.cpp_info.libs = ["cloe-osi"]
        else:
            self.cpp_info.libs = files.collect_libs(self)
