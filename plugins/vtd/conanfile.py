from pathlib import Path
from conans import CMake, ConanFile, tools


class CloeSimulatorVTD(ConanFile):
    name = "cloe-plugin-vtd"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe simulator plugin that binds to Vires VTD"
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
        "bin/*",
        "cmake/*",
        "src/*",
        "CMakeLists.txt",
        "module.py",
    ]
    no_copy_source = True
    requires = [
        "open-simulation-interface/3.2.0@cloe/stable",
        "vtd-api/2.2.0@cloe/stable",
    ]

    _cmake = None

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

    def configure(self):
        self.options["open-simulation-interface"].shared = False
        self.options["open-simulation-interface"].fPIC = True

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy("vtd-launch", dst="bin", src=f"{self.source_folder}/bin")

    def package_id(self):
        # Changes in a dependency's package_id need to be accounted for since
        # the package statically links against vtd-object-lib.
        self.info.requires["cloe-models"].full_package_mode()
        self.info.requires["cloe-runtime"].full_package_mode()
        self.info.requires["open-simulation-interface"].full_package_mode()
        self.info.requires["vtd-api"].full_package_mode()

    def package_info(self):
        self.env_info.VTD_LAUNCH = f"{self.package_folder}/bin/vtd-launch"
