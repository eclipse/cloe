# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path
from conans import CMake, ConanFile, tools


class CloeComponentKmph(ConanFile):
    name = "cloe-plugin-speedometer"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe component plugin providing a speedometer trigger"
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

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../../VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

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

    def package_id(self):
        del self.info.options.pedantic
