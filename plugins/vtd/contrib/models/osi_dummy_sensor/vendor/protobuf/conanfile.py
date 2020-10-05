from conans import ConanFile, CMake, tools
import os


def _add_include_path(path):
    return " -isystem " + path


class ProtobufConan(ConanFile):
    name = "protobuf"
    version = "2.6.1"
    default_user = "cloe"
    default_channel = "stable"
    license = "BSD-3-Clause"
    url = "https://github.com/protocolbuffers/protobuf.git"
    description = "Google Protocol Buffers."
    topics = ("protobuf", "serialization")
    settings = "os", "compiler", "build_type", "arch"
    build_policy = "missing"
    _patch_v2p6p1 = "patches/protobuf_v2.6.1.patch"  # git format-patch v2.6.1 --stdout > protobuf_v2.6.1.patch
    exports_sources = _patch_v2p6p1
    _src_dir = "protobuf"
    _pkg_lib_dir = "lib/"
    _cpp_flag = ""  # will be set during build

    def source(self):
        self.run("git clone --depth 1 --single-branch --branch v" + self.version +
                 " https://github.com/protocolbuffers/protobuf.git " + self._src_dir)

    def configure(self):
        # package options
        if self.develop:
            tools.check_min_cppstd(self, "11")
        else:
            self.settings.compiler.cppstd = "14"

    def build(self):
        # apply patch to the sources that have been copied to the build folder
        patch_path = os.path.join(self.source_folder, self._src_dir)
        tools.patch(base_path=patch_path, patch_file=self._patch_v2p6p1)
        self.run("cd {0:} && ./autogen.sh".format(self._src_dir))
        # set cxx flags
        cpp_std = "{0:}".format(self.settings.get_safe("compiler.cppstd"))
        self._cpp_flag = "-std=c++" + cpp_std if cpp_std.isnumeric() else "-std=" + "{0:}".format(
            cpp_std.replace("gnu", "gnu++"))
        cxx_flags = self._cpp_flag
        # use abi setting as requested
        if self.settings.get_safe("compiler.libcxx") == "libstdc++":
            cxx_flags += " -D_GLIBCXX_USE_CXX11_ABI=0"
        # skip static libraries
        options = "--enable-static=no "
        # configure
        self.run("cd {0:} && ./configure --prefix={1:} {2:} CXXFLAGS='{3:}'".format(
            self._src_dir, self.package_folder, options, cxx_flags))
        # build
        self.run("cd {0:} && make".format(self._src_dir))

    def package(self):
        self.run("cd {0:} && make install".format(self._src_dir))

    def package_info(self):
        self.cpp_info.libdirs = [self._pkg_lib_dir]
        self.cpp_info.cxxflags = [self._cpp_flag]
        self.cpp_info.libs = tools.collect_libs(self)
