from conans import ConanFile, AutoToolsBuildEnvironment, tools
import os


class ProtobufConan(ConanFile):
    name = "protobuf"
    version = "2.6.1"
    license = "BSD-3-Clause"
    url = "https://github.com/protocolbuffers/protobuf.git"
    description = "Google Protocol Buffers."
    topics = ("protobuf", "serialization")
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    build_policy = "missing"
    _patch_v2p6p1 = "patches/protobuf_v2.6.1.patch"  # git format-patch v2.6.1 --stdout > protobuf_v2.6.1.patch
    exports_sources = [
        _patch_v2p6p1,
        "CMakeLists.txt",
    ]
    build_requires = [
        "autoconf/[>=2.69]",
        "libtool/[>=2.4.6]",
    ]
    _src_dir = "protobuf"
    _git_url = url
    _git_ref = "v{}".format(version)
    _git_dir = "protobuf"
    _autotools = None

    def source(self):
        git = tools.Git(folder=self._git_dir)
        git.clone(self._git_url, branch=self._git_ref, shallow=True)

    def build(self):
        # change googletest version
        tools.patch(base_path=self._src_dir, patch_file=self._patch_v2p6p1)
        self.run("cd {0:} && ./autogen.sh".format(self._src_dir))
        # configure and make
        self._autotools = AutoToolsBuildEnvironment(self)
        self._autotools.configure(configure_dir=self._src_dir)
        self._autotools.make()

    def package(self):
        self._autotools.make(target="install")

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
