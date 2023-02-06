# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path
from conans import CMake, ConanFile, RunEnvironment, tools


class CloeEngine(ConanFile):
    name = "cloe-engine"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Cloe engine to execute simulations"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        # Whether the server feature is compiled and built into the Cloe engine.
        # Not building this may make compiling the engine possible if the web
        # server dependencies are incompatible with your target system.
        "server": [True, False],

        # Make the compiler be strict and pedantic.
        # Disable if you upgrade compilers and run into new warnings preventing
        # the build from completing. May be removed in the future.
        "pedantic": [True, False],
    }
    default_options = {
        "server": True,
        "pedantic": True,

        "fable:allow_comments": True,
    }
    generators = "cmake"
    no_copy_source = True
    exports_sources = [
        "src/*",
        "webui/*",
        "CMakeLists.txt",
    ]

    _cmake = None

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-runtime/{self.version}@cloe/develop")
        self.requires(f"cloe-models/{self.version}@cloe/develop")
        self.requires("cli11/[~=2.1.2]", private=True)
        if self.options.server:
            self.requires(f"cloe-oak/{self.version}@cloe/develop", private=True)
            self.requires("boost/[>=1.65.1,<1.70.0]")
        else:
            self.requires("boost/[>=1.65.1]")
        self.requires("fmt/[~=8.1.1]", override=True)
        self.requires("nlohmann_json/[~=3.10.5]", override=True)

    def build_requirements(self):
        self.test_requires("gtest/[~1.10]")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["WithServer"] = self.options.server
        self._cmake.definitions["TargetLintingExtended"] = self.options.pedantic
        self._cmake.definitions["CLOE_PROJECT_VERSION"] = self.version
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        with tools.environment_append(RunEnvironment(self).vars):
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        del self.info.options.pedantic

    def package_info(self):
        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("pthread")
            self.cpp_info.system_libs.append("dl")
        if self.in_local_cache:
            bindir = os.path.join(self.package_folder, "bin")
        else:
            bindir = os.path.join(self.package_folder, "build", "bin")
        self.output.info(f"Appending PATH environment variable: {bindir}")
        self.env_info.PATH.append(bindir)
