from pathlib import Path
from conans import CMake, ConanFile, RunEnvironment, tools


class Cloe(ConanFile):
    name = "cloe"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Closed-loop automated driving simulation environment"
    topics = ["simulation"]
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "test": [True, False],
        "pedantic": [True, False],
        "with_vtd": [True, False],
        "with_engine": [True, False],
    }
    default_options = {
        "test": True,
        "pedantic": True,
        "with_vtd": False,
        "with_engine": True,
    }
    generators = "cmake"
    no_copy_source = True

    _cmake = None

    def set_version(self):
        version_file = Path(self.recipe_folder) / "VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires("boost/[~=1.69]", override=True)
        self.requires("fmt/[~=8.1.1]")
        requires = [
            "cloe-runtime",
            "cloe-models",
            "cloe-plugin-basic",
            "cloe-plugin-gndtruth-extractor",
            "cloe-plugin-minimator",
            "cloe-plugin-mocks",
            "cloe-plugin-noisy-sensor",
            "cloe-plugin-speedometer",
            "cloe-plugin-virtue",
        ]
        if self.options.with_vtd:
            requires.append("cloe-plugin-vtd")

        if self.options.with_engine:
            requires.append("cloe-engine")

        for dep in requires:
            self.requires(f"{dep}/{self.version}@cloe/develop")

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
        # This build is for a workspace build. See: conanws.yml
        if not self.in_local_cache:
            cmake = self._configure_cmake()
            cmake.build()
            if self.options.test:
                with tools.environment_append(RunEnvironment(self).vars):
                    cmake.test()

    def package(self):
        # This build is for a workspace build. See: conanws.yml
        if not self.in_local_cache:
            cmake = self._configure_cmake()
            cmake.install()
