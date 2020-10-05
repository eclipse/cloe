import os.path
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from shutil import which


class CloeControllerBasic(ConanFile):
    name = "cloe-plugin-basic"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe controller plugin providing basic ACC"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "test": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "test": True,
        "pedantic": True,
    }
    generators = "cmake"
    exports_sources = [
        "src/*",
        "ui/*",
        "CMakeLists.txt",
    ]
    no_copy_source = True

    _cmake = None

    def _project_version(self):
        version_file = os.path.join(self.recipe_folder, "..", "..", "VERSION")
        return tools.load(version_file)

    def set_version(self):
        self.version = self._project_version()

    def requirements(self):
        self.requires("cloe-runtime/{}@cloe/develop".format(self.version))
        self.requires("cloe-models/{}@cloe/develop".format(self.version))

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
