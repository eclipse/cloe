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
        "with_vtd": [True, False],
        "with_engine": [True, False],

        # Doesn't affect package ID:
        "test": [True, False],
        "pedantic": [True, False],
    }
    default_options = {
        "with_vtd": False,
        "with_engine": True,

        "test": True,
        "pedantic": True,

        "cloe-engine:server": True,
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
        def cloe_requires(dep):
            self.requires(f"{dep}/{self.version}@cloe/develop")

        cloe_requires("cloe-runtime")
        cloe_requires("cloe-models")
        cloe_requires("cloe-plugin-basic")
        cloe_requires("cloe-plugin-gndtruth-extractor")
        cloe_requires("cloe-plugin-minimator")
        cloe_requires("cloe-plugin-mocks")
        cloe_requires("cloe-plugin-noisy-sensor")
        cloe_requires("cloe-plugin-speedometer")
        cloe_requires("cloe-plugin-virtue")
        if self.options.with_vtd:
            cloe_requires("cloe-plugin-vtd")

        boost_version = "[>=1.65.0]"
        if self.options.with_engine:
            cloe_requires("cloe-engine")
            if self.options["cloe-engine"].server:
                boost_version = "[<1.70]"

        # Overrides:
        self.requires("fmt/[~=8.1.1]", override=True)
        self.requires("inja/[~=3.3.0]", override=True)
        self.requires("nlohmann_json/[~=3.10.5]", override=True)
        self.requires("incbin/[~=0.88.0]@cloe/stable", override=True),
        self.requires(f"boost/{boost_version}", override=True)

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

    def package_id(self):
        del self.info.options.test
        del self.info.options.pedantic
