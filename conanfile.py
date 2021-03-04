import os.path
from conans import ConanFile, CMake, tools


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
    no_copy_source = False

    _cmake = None

    def _project_version(self):
        version_file = os.path.join(self.recipe_folder, "VERSION")
        return tools.load(version_file).strip()

    def set_version(self):
        self.version = self._project_version()

    def _requires(self, name):
        self.requires("{}/{}@cloe/develop".format(name, self.version))

    def requirements(self):
        self.requires("boost/[<1.70]", override=True)
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
            self.requires("vtd-osi-sensor/1.0.0@cloe/vtd2p2")

        if self.options.with_engine:
            requires.append("cloe-engine")

        for dep in requires:
            self._requires(dep)

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
                cmake.test()

    def package(self):
        # This build is for a workspace build. See: conanws.yml
        if not self.in_local_cache:
            cmake = self._configure_cmake()
            cmake.install()
