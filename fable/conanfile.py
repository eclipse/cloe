# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path
from conans import CMake, ConanFile, RunEnvironment, tools


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
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "include/*",
        "examples/*",
        "src/*",
        "CMakeLists.txt",
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
        self.requires("boost/[>=1.65.1]")
        self.requires("fmt/[>=6.2.0]")
        self.requires("nlohmann_json/[~=3.10.5]")

    def build_requirements(self):
        self.test_requires("gtest/[~=1.10]")
        self.test_requires("cli11/[~2.1.2]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["FABLE_VERSION"] = self.version
        self._cmake.definitions["AllowComments"] = self.options.allow_comments
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
        if self.in_local_cache:
            self.cpp_info.libs = tools.collect_libs(self)
        else:
            self.cpp_info.libs = ["fable"]
