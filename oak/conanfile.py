# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, files, scm, env

required_conan_version = ">=1.52.0"

class CloeOak(ConanFile):
    name = "cloe-oak"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe web-server backend implementation"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "shared": False,
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
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires("oatpp/1.3.0")

    def build_requirements(self):
        self.test_requires("gtest/1.14.0")

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
        del self.info.options.pedantic

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe-oak")
        self.cpp_info.set_property("cmake_target_name", "cloe::oak")
        self.cpp_info.set_property("pkg_config_name", "cloe-oak")

        # Linking to libstdc++fs is required on GCC < 9.
        # (GCC compilers with version < 7 have no std::filesystem support.)
        # No consideration has been made yet for other compilers,
        # please add them here as necessary.
        if self.settings.get_safe("compiler") == "gcc" and self.settings.get_safe("compiler.version") in ["7", "8"]:
            self.cpp_info.system_libs = ["stdc++fs"]

        # Make sure we can find the library, both in editable mode and in the
        # normal package mode:
        if not self.in_local_cache:
            self.cpp_info.libs = ["cloe-oak"]
        else:
            self.cpp_info.libs = files.collect_libs(self)
