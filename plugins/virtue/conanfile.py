import os.path
from conans import ConanFile, CMake, tools


class CloeControllerVirtue(ConanFile):
    name = "cloe-plugin-virtue"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe controller plugin that verifies basic assumptions"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "pedantic": [True, False],
    }
    default_options = {
        "pedantic": True,
    }
    generators = "cmake"
    exports_sources = [
        "src/*",
        "CMakeLists.txt",
    ]
    no_copy_source = True

    _cmake = None

    def _project_version(self):
        version_file = os.path.join(self.recipe_folder, "..", "..", "VERSION")
        return tools.load(version_file).strip()

    def set_version(self):
        self.version = self._project_version()

    def requirements(self):
        self.requires("cloe-runtime/{}@cloe/develop".format(self.version))
        self.requires("cloe-models/{}@cloe/develop".format(self.version))

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
