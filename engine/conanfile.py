# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, env, files, scm

required_conan_version = ">=1.52.0"

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
    generators = "CMakeDeps", "VirtualRunEnv"
    no_copy_source = True
    exports_sources = [
        "src/*",
        "webui/*",
        "CMakeLists.txt",
    ]

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
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

    def layout(self):
        cmake.cmake_layout(self)

    def generate(self):
        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CLOE_PROJECT_VERSION"] = self.version
        tc.cache_variables["CLOE_ENGINE_WITH_SERVER"] = self.options.server
        tc.cache_variables["TargetLintingExtended"] = self.options.pedantic
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        cm.configure()
        cm.build()
        cm.test()

    def package(self):
        cm = cmake.CMake(self)
        cm.install()

    def package_id(self):
        self.info.requires["boost"].full_package_mode()
        del self.info.options.pedantic

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe-engine")
        self.cpp_info.set_property("cmake_target_name", "cloe::engine")
        self.cpp_info.set_property("pkg_config_name", "cloe-engine")
        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("pthread")
            self.cpp_info.system_libs.append("dl")
        if self.in_local_cache:
            bindir = os.path.join(self.package_folder, "bin")
        else:
            bindir = os.path.join(self.build_folder, str(self.settings.build_type), "bin")
        self.output.info(f"Appending PATH environment variable: {bindir}")
        self.env_info.PATH.append(bindir)
