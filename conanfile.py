# mypy: ignore-errors
# pylint: skip-file

import os
from pathlib import Path
from semver import SemVer

from conan import ConanFile
from conan.tools import cmake, files, scm

required_conan_version = ">=1.52.0"


class Cloe(ConanFile):
    name = "cloe"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Closed-loop automated driving simulation environment"
    topics = ["simulation"]
    settings = "os", "compiler", "build_type", "arch"
    provides = (
        "fable",
        "cloe-runtime",
        "cloe-models",
        "cloe-oak",
        "cloe-engine",
        "cloe-plugins-core",
        "cloe-plugin-basic",
        "cloe-plugin-gndtruth-extractor",
        "cloe-plugin-minimator",
        "cloe-plugin-mocks",
        "cloe-plugin-noisy-sensor",
        "cloe-plugin-speedometer",
        "cloe-plugin-virtue",
    )
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "fable_allow_comments": [True, False],
        "engine_server": [True, False],
        "engine_lrdb": [True, False]
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "fable_allow_comments": True,
        "engine_server": True,
        "engine_lrdb": True,
    }
    generators = "CMakeDeps", "VirtualRunEnv"
    no_copy_source = True
    exports_sources = [
        "*/cmake/*",
        "*/src/*",
        "*/include/*",
        "*/CMakeLists.txt",

        "fable/examples/*",

        "engine/lua/*",
        "engine/webui/*",
        "engine/vendor/*",

        "plugins/*/src/*",
        "plugins/*/include/*",
        "plugins/*/ui/*",
        "plugins/*/CMakeLists.txt",

        "CMakelists.txt"
    ]

    def set_version(self):
        version_file = Path(self.recipe_folder) / "VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires("fmt/9.1.0")
        self.requires("inja/3.4.0")
        self.requires("nlohmann_json/3.11.2")
        self.requires("incbin/cci.20211107"),
        self.requires("spdlog/1.11.0")
        self.requires("eigen/3.4.0")
        self.requires("cli11/2.3.2", private=True)
        self.requires("sol2/3.3.1")
        self.requires("boost/[>=1.65.1]")
        if self.options.engine_server:
            self.requires("oatpp/1.3.0")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")

    def layout(self):
        cmake.cmake_layout(self)
        self.cpp.build.bindirs = ["bin"]
        self.cpp.source.includedirs.append(os.path.join(self.folders.build, "include"))

    def generate(self):
        # The version as a single 32-bit number takes the format:
        #
        #   (EPOCH << 24) | (MAJOR_VERSION << 16) | (MINOR_VERSION << 8) | PATCH_VERSION
        #
        # Each version consists of at most 8 bits, so 256 potential values, including 0.
        # The epoch starts with 0, and is bumped after each version naming scheme.
        semver = SemVer(self.version, True)
        version_u32 = (0<<24) | (semver.major << 16) | (semver.minor << 8) | semver.patch

        tc = cmake.CMakeToolchain(self)
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.cache_variables["CMAKE_MODULE_PATH"] = self.source_folder + "/runtime/cmake"
        tc.cache_variables["FABLE_VERSION"] = self.version
        tc.cache_variables["FABLE_VERSION_U32"] = version_u32
        tc.cache_variables["FABLE_ALLOW_COMMENTS"] = self.options.fable_allow_comments
        tc.cache_variables["CLOE_PROJECT_VERSION"] = self.version
        tc.cache_variables["CLOE_VERSION"] = self.version
        tc.cache_variables["CLOE_VERSION_U32"] = version_u32
        tc.cache_variables["CLOE_ENGINE_WITH_SERVER"] = self.options.engine_server
        tc.cache_variables["CLOE_ENGINE_WITH_LRDB"] = self.options.engine_lrdb
        tc.generate()

    def build(self):
        cm = cmake.CMake(self)
        if self.should_configure:
            cm.configure()
        if self.should_build:
            cm.build()
        if self.should_test:
            cm.test()

    def package(self):
        if self.should_install:
            cm = cmake.CMake(self)
            cm.install()

            # Package license files for compliance
            for meta, dep in self.dependencies.items():
                if dep.package_folder is None:
                    continue
                ref = str(meta.ref)
                name = ref[: str(ref).index("/")]
                files.copy(
                    self,
                    "*",
                    src=os.path.join(dep.package_folder, "licenses"),
                    dst=os.path.join(self.package_folder, "licenses", name),
                )

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "cloe")
        self.cpp_info.set_property("pkg_config_name", "cloe")

        self.cpp_info.components["fable"].libs = ["fable"]
        self.cpp_info.components["fable"].set_property("cmake_file_name", "fable")
        self.cpp_info.components["fable"].set_property("cmake_target_name", "fable::fable")
        self.cpp_info.components["fable"].set_property("pkg_config_name", "fable")

        self.cpp_info.components["runtime"].libs = ["cloe-runtime"]
        self.cpp_info.components["runtime"].requires = ["fable"]
        self.cpp_info.components["runtime"].set_property("cmake_file_name", "cloe-runtime")
        self.cpp_info.components["runtime"].set_property("cmake_target_name", "cloe::runtime")
        self.cpp_info.components["runtime"].set_property("pkg_config_name", "cloe-runtime")

        if self.settings.os == "Linux":
            self.cpp_info.system_libs.append("pthread")
            self.cpp_info.system_libs.append("dl")

        # Linking to libstdc++fs is required on GCC < 9.
        # (GCC compilers with version < 7 have no std::filesystem support.)
        # No consideration has been made yet for other compilers,
        # please add them here as necessary.
        if self.settings.get_safe("compiler") == "gcc" and self.settings.get_safe("compiler.version") in ["7", "8"]:
            self.cpp_info.system_libs = ["stdc++fs"]

        self.cpp_info.libs = files.collect_libs(self)
        if not self.in_local_cache: # editable build
            self.cpp_info.builddirs.append(os.path.join(self.source_folder, "cmake"))
            self.cpp_info.includedirs.append(os.path.join(self.build_folder, "include"))
            bindir = os.path.join(self.build_folder, "bin")
            luadir = os.path.join(self.source_folder, "engine/lua")
            libdir = os.path.join(self.build_folder, "lib");
        else:
            self.cpp_info.builddirs.append(os.path.join("lib", "cmake", "cloe"))
            bindir = os.path.join(self.package_folder, "bin")
            luadir = os.path.join(self.package_folder, "lib/cloe/lua")
            libdir = None

        self.output.info(f"Appending PATH environment variable: {bindir}")
        self.runenv_info.prepend_path("PATH", bindir)
        self.output.info(f"Appending CLOE_LUA_PATH environment variable: {luadir}")
        self.runenv_info.prepend_path("CLOE_LUA_PATH", luadir)
        if libdir is not None:
            self.output.info(f"Appending LD_LIBRARY_PATH environment variable: {libdir}")
            self.runenv_info.append_path("LD_LIBRARY_PATH", libdir)
