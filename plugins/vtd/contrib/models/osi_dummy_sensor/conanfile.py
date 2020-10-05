from conans import ConanFile, CMake, tools
import os


def _add_include_path(path):
    return " -isystem " + path


class OsiDummySensorConan(ConanFile):
    name = "osi-dummy-sensor"
    version = "1.0.0"
    default_user = "cloe"
    default_channel = "stable"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/OpenSimulationInterface/osi-sensor-model-packaging"
    description = "Examples of how sensor models are to be packaged for use in simulation environments with FMI 2.0."
    topics = ("Sensor Simulation", "HAD")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "link_osi_shared": [True, False],  # typically shared for vtd
        "osi_version": "ANY",
        "osi_lib_path": "ANY",  # osi lib provided by vtd
        "link_trg_path": "ANY"
    }  # directory where to create a symbolic link for convenience
    default_options = {"link_osi_shared": True, "osi_version": "[>=3.0.0]"}
    generators = "cmake"
    build_policy = "missing"
    _patch_vtd2p2 = "patches/osmp_vtd_v2.2.patch"  # git format-patch v1.0.0 --stdout > osmp_vtd_v2.2.patch
    exports_sources = _patch_vtd2p2, "include/*"  # osi and protobuf v2.6.1 header files are needed for vtd v2.2.
    _src_dir_osmp = "osmp"
    _pkg_lib_dir = "lib/"
    _pkg_lib = "OSMPDummySensor.so"
    _cpp_flag = ""  # will be set during build

    def requirements(self):
        self.requires("open-simulation-interface/{0:}@cloe/stable".format(self.options.osi_version))

    def source(self):
        self.run("git clone --depth 1 --single-branch --branch v" + self.version +
                 " https://github.com/OpenSimulationInterface/osi-sensor-model-packaging.git " +
                 self._src_dir_osmp)

    def configure(self):
        # requirement options
        self.options["open-simulation-interface"].shared = self.options.link_osi_shared
        # package options
        if self.develop:
            tools.check_min_cppstd(self, "11")
        else:
            self.settings.compiler.cppstd = "14"

    def build(self):
        options = {}
        # apply patch to the sources that have been copied to the build folder
        patch_path = os.path.join(self.source_folder, self._src_dir_osmp)
        tools.patch(base_path=patch_path, patch_file=self._patch_vtd2p2)
        # set cxx flags
        cmake = CMake(self)
        cpp_std = "{0:}".format(self.settings.get_safe("compiler.cppstd"))
        self._cpp_flag = "-std=c++" + cpp_std if cpp_std.isnumeric() else "-std=" + "{0:}".format(
            cpp_std.replace("gnu", "gnu++"))
        cxx_flags = self._cpp_flag
        # use abi setting as requested
        if self.settings.get_safe("compiler.libcxx") == "libstdc++":
            cxx_flags += " -D_GLIBCXX_USE_CXX11_ABI=0"
        # set include paths
        cxx_flags += _add_include_path(patch_path + "/examples/includes/")
        for inc in self.deps_cpp_info["open-simulation-interface"].include_paths:
            cxx_flags += _add_include_path(inc)
        for inc in self.deps_cpp_info["protobuf"].include_paths:
            cxx_flags += _add_include_path(inc)
        options["CMAKE_CXX_FLAGS"] = "'" + cxx_flags + "'"
        options["CMAKE_BUILD_TYPE"] = self.settings.get_safe("build_type")
        osi_link_path = ""
        inc = "{0:}".format(self.options.osi_lib_path)
        osi_link_path += " -L " + inc
        # set osi linking options
        if self.options.link_osi_shared:
            linker_flag_name = "CMAKE_SHARED_LINKER_FLAGS"
            osi_shared_linking = "ON"
        else:
            linker_flag_name = "CMAKE_STATIC_LINKER_FLAGS"
            osi_shared_linking = "OFF"
        options[linker_flag_name] = "'" + osi_link_path + "'"
        options["LINK_WITH_SHARED_OSI"] = osi_shared_linking
        src_path = os.path.join(self._src_dir_osmp, "examples/OSMPDummySensor")
        cmake.configure(defs=options, source_folder=src_path)
        cmake.build()

    def package(self):
        self.copy(self._pkg_lib, dst=self._pkg_lib_dir, keep_path=False)

    def package_info(self):
        self.cpp_info.libdirs = [self._pkg_lib_dir]
        self.cpp_info.cxxflags = [self._cpp_flag]
        self.cpp_info.libs = tools.collect_libs(self)
        # will be called even if the package is already in cache
        pkg_path = os.path.join(self._pkg_lib_dir, self._pkg_lib)
        pkg_path = os.path.join(self.package_folder, pkg_path)
        trg_path = os.path.join("{0:}".format(self.options.link_trg_path), self._pkg_lib)
        if os.path.islink(trg_path):
            os.unlink(trg_path)
        os.symlink(pkg_path, trg_path)
