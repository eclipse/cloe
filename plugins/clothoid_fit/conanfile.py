from pathlib import Path
from conans import ConanFile, CMake, RunEnvironment, tools


class CloeComponentClothoidFit(ConanFile):
    name = "cloe-plugin-clothoid-fit"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe component plugin that generates clothoids from given lane boundary point lists."
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
    no_copy_source = True
    exports_sources = [
        "src/*",
        "CMakeLists.txt",
    ]
    requires = [
        "eigen/[~=3.4.0]",
    ]

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

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            with tools.environment_append(RunEnvironment(self).vars):
                cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_id(self):
        del self.info.options.pedantic
        del self.info.options.test
