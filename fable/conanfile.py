# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

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
        self.requires("boost/[>=1.65.1]")
        self.requires("fmt/[>=6.2.0]")
        self.requires("nlohmann_json/[~=3.10.5]")

    def build_requirements(self):
        self.test_requires("gtest/[~=1.10]")

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["FABLE_VERSION"] = self.version
        tc.cache_variables["FABLE_ALLOW_COMMENTS"] = self.options.allow_comments
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

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "fable")
        self.cpp_info.set_property("cmake_target_name", "fable::fable")
        self.cpp_info.set_property("pkg_config_name", "fable")
        # TODO: Does this also work in editable mode?
        self.cpp_info.libs = files.collect_libs(self)
