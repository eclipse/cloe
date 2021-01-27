from conans import ConanFile, CMake, tools
from pathlib import Path
import shutil
import os


class VtdSensorConan(ConanFile):
    name = "vtd-osi-sensor"
    version = "1.0.0"
    default_user = "cloe"
    default_channel = "vtd2p2"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/OpenSimulationInterface/osi-sensor-model-packaging"
    description = "Example of a sensor model using OSI with FMI 2.0."
    topics = ("Sensor Simulation", "HAD")
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    build_policy = "missing"
    _patch_vtd2p2 = "patches/osmp_vtd_v2.2.patch"  # git format-patch v1.0.0 --stdout > osmp_vtd_v2.2.patch
    exports_sources = [
        _patch_vtd2p2,
        "CMakeLists.txt",
    ]
    _git_url = "https://github.com/OpenSimulationInterface/osi-sensor-model-packaging.git"
    _git_ref = "v{}".format(version)
    _git_dir = "osmp"
    _rel_src_dir = os.path.join(_git_dir, "examples/OSMPDummySensor")
    _pkg_lib = "OSMPDummySensor.so"
    _cmake = None

    def requirements(self):
        self.requires("open-simulation-interface/[=3.0.*]@cloe/vtd2p2",
                      private=True)
        self.requires("protobuf/2.6.1@cloe/vtd2p2", override=True)
        self.requires("vtd/2.2.0@cloe/stable")

    def source(self):
        git = tools.Git(folder=self._git_dir)
        git.clone(self._git_url, branch=self._git_ref, shallow=True)
        dst = os.path.join(self.source_folder, self._rel_src_dir)
        shutil.copy("CMakeLists.txt", dst)

    def configure(self):
        # requirement options
        self.options["open-simulation-interface"].shared = True
        self.options["open-simulation-interface"].protoc_from_protobuf = True
        # package options
        self.settings.compiler.libcxx = "libstdc++"

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_PROJECT_VERSION"] = self.version
        self._cmake.definitions["CMAKE_BUILD_TYPE"] = self.settings.get_safe(
            "build_type")
        self._cmake.definitions["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        self._cmake.configure(source_folder=self._rel_src_dir)
        return self._cmake

    def build(self):
        # find VTD osi library
        osi_lib_dir = Path(self.deps_env_info["vtd"].VTD_ROOT +
                           "/Data/Setups/Standard.OSI3/Bin/")
        if not Path(str(osi_lib_dir) +
                    "/libopen_simulation_interface.so").is_file():
            self.output.warn(
                "VTD OSI library not found: {}".format(osi_lib_dir))
        # apply patch for compatibility with VTD osi plugin
        tools.patch(base_path=self._git_dir, patch_file=self._patch_vtd2p2)
        # configure and build
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        pkg_path = os.path.join(self.package_folder, "lib", self._pkg_lib)
        # collect the library path for linking the model to the VTD runtime setup
        self.env_info.VTD_EXTERNAL_MODELS.append(pkg_path)
