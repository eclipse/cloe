import os.path

from conans import CMake, ConanFile, tools


class CloeSimulatorMinimator(ConanFile):
    name = "cloe-plugin-minimator"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe simulator plugin that is very minimalistic"
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
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
