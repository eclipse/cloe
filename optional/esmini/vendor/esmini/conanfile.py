import glob
import os
from pathlib import Path
from conans import CMake, ConanFile, RunEnvironment, tools
from conans.tools import SystemPackageTool


class ESMini(ConanFile):
    name = "esmini"
    version = "2.24.0"
    license = "Mozilla Public License Version 2.0"
    url = "https://github.com/esmini/esmini"
    description = "Basic OpenScenario player."
    topics = ("Environment Simulator", "OpenScenario", "OpenDrive")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "test": [True, False],
        "with_osg": [True, False],
        "with_osi": [True, False],
        "with_sumo": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "test": True,
        "with_osg": True,
        "with_osi": True,
        "with_sumo": False,
    }
    generators = "cmake"
    build_policy = "missing"
    no_copy_source = False
    requires = []

    _patch_file = "patches/esmini_2_25_1.patch"

    _pkg_scenario_dir = "scenarios"

    exports_sources = [
        "CMakeLists.txt",
        _patch_file,
        f"{_pkg_scenario_dir}/*",
    ]
    _git_url = "https://github.com/esmini/esmini.git"
    _git_dir = "esmini"
    _git_ref = "develop" if version == "latest" else f"v{version}"
    _sim_dir = "EnvironmentSimulator"
    _test_deps = [_sim_dir, "resources", "scripts"]
    _test_dir = f"{_git_dir}/{_sim_dir}/Unittest/"
    _lib_dir = f"{_git_dir}/{_sim_dir}/Libraries/"
    _bin_dir = f"{_git_dir}/{_sim_dir}/Applications/"
    _resources_dir = f"{_git_dir}/resources"

    _protobuf_dyn = True

    _cmake = None

    def configure(self):
        if self.options.with_osg:
            self.options.with_osi = True
        if self.options.with_osi:
            self.options["open-simulation-interface"].shared = self.options.shared
            self.options["protobuf"].shared = self._protobuf_dyn
            self.options["protobuf"].debug_suffix = False

    def source(self):
        git = tools.Git(folder=self._git_dir)
        git.clone(self._git_url, self._git_ref, shallow=True)

    def system_requirements(self):
        pkg_names = None
        if self.options.with_osg:
            pkg_names = [
                "libfontconfig1-dev",
                "libgl-dev",
                "libxrandr-dev",
                "libxinerama-dev",
            ]  # TODO: add all system requirements
        if pkg_names:
            installer = SystemPackageTool()
            for pkg in pkg_names:
                installer.install([pkg])

    def requirements(self):
        if self.options.with_osi:
            self.requires("protobuf/[~=3.15.5]", override=True)
            self.requires("open-simulation-interface/3.3.1@cloe/stable")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_PROJECT_SUBDIR"] = self._git_dir
        self._cmake.definitions["CMAKE_PROJECT_VERSION"] = self.version
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.definitions["BUILD_SHARED_LIBS"] = self.options.shared
        self._cmake.definitions["USE_OSG"] = self.options.with_osg
        self._cmake.definitions["USE_OSI"] = self.options.with_osi
        self._cmake.definitions["USE_SUMO"] = self.options.with_sumo
        self._cmake.definitions["USE_GTEST"] = self.options.test
        self._cmake.definitions["DYN_PROTOBUF"] = self._protobuf_dyn
        self._cmake.configure()
        return self._cmake

    def build(self):
        trg_path = self._git_dir
        patch_file = self._patch_file
        if not self.in_local_cache:
            trg_path = self.source_folder + "/" + trg_path
            patch_file = self.recipe_folder + "/" + patch_file
        tools.patch(base_path=trg_path, patch_file=patch_file)

        cmake = self._configure_cmake()
        cmake.build()
        if self.options.test:
            self._prepare_tests()
            with tools.chdir(self._test_dir):
                with tools.environment_append(RunEnvironment(self).vars):
                    for test in glob.glob("*_test"):
                        self.run(Path(test).resolve().as_posix())
            self._cleanup_tests()

    def _prepare_tests(self):
        for d in self._test_deps:
            self.run(f"ln -sf {self.source_folder}/{self._git_dir}/{d}")

    def _cleanup_tests(self):
        for d in self._test_deps:
            if os.path.islink(d):
                self.run(f"rm -f {d}")

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy(
            pattern="*.hpp",
            dst="include",
            src=f"{self.source_folder}/{self._lib_dir}",
            keep_path=False,
        )
        self.copy(pattern="*.so", dst="lib", src=self._lib_dir, keep_path=False)
        self.copy(pattern="*.a", dst="lib", src=self._lib_dir, keep_path=False)
        apps = os.listdir(self._bin_dir)
        for app in apps:
            self.copy(
                pattern=app, dst="bin", src=f"{self._bin_dir}/{app}", keep_path=False
            )
        self.copy(
            pattern="*",
            dst=self._pkg_scenario_dir,
            src=f"{self.source_folder}/{self._resources_dir}",
        )
        assert Path(f"{self.package_folder}/{self._pkg_scenario_dir}/xosc").exists()
        self.copy(
            "*",
            dst=self._pkg_scenario_dir,
            src=f"{self.source_folder}/{self._pkg_scenario_dir}",
        )

    def package_id(self):
        if self.options.with_osi:
            self.info.requires["open-simulation-interface"].full_package_mode()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", self.name)
        self.cpp_info.set_property("pkg_config_name", self.name)

        self.cpp_info.libs = tools.collect_libs(self)
        self.runenv_info.define("ESMINI_XOSC_PATH", f"{self.package_folder}/{self._pkg_scenario_dir}/xosc")
