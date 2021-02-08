import os.path
from conans import ConanFile, CMake, tools


class CloeEngine(ConanFile):
    name = "cloe-engine"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe engine to execute simulations"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "test": [True, False],
        "pedantic": [True, False],
        "boost_version": "ANY",
    }
    default_options = {
        "test": True,
        "pedantic": True,
        "boost_version": "[>=1.65.0 <1.70]",
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "src/*",
        "webui/*",
        "CMakeLists.txt",
    ]
    build_requires = [
        "cli11/1.9.1",
    ]

    _cmake = None

    def _project_version(self):
        version_file = os.path.join(self.recipe_folder, "..", "VERSION")
        return tools.load(version_file).strip()

    def set_version(self):
        self.version = self._project_version()

    def build_requirements(self):
        if self.options.test:
            self.build_requires("gtest/[~1.10]")

    def requirements(self):
        # Enforce CppNetlib boost requirement for all upstream packages.
        # Note that boost is not interpreted as an actual requirement of cloe-engine.
        self.requires("boost/{}".format(self.options.boost_version), override=True)
        self.requires("cloe-runtime/{}@cloe/develop".format(self.version),private=True)
        self.requires("cloe-models/{}@cloe/develop".format(self.version),private=True)
        self.requires("cloe-oak/{}@cloe/develop".format(self.version),private=True)

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["BuildTests"] = self.options.test
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.definitions["CLOE_PROJECT_VERSION"] = self.version
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

    def package_info(self):
        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("pthread")
            self.cpp_info.system_libs.append("dl")
        bindir = os.path.join(self.package_folder, "bin")
        self.output.info("Appending PATH environment variable: {}".format(bindir))
        self.env_info.PATH.append(bindir)
