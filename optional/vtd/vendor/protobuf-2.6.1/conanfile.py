# mypy: ignore-errors
# pylint: skip-file

import os

from conan import ConanFile
from conan.tools import scm, gnu, files

required_conan_version = ">=1.52.0"


class ProtobufConan(ConanFile):
    name = "protobuf"
    version = "2.6.1"
    license = "BSD-3-Clause"
    url = "https://github.com/protocolbuffers/protobuf.git"
    description = "Google Protocol Buffers."
    topics = ("protobuf", "serialization")
    settings = "os", "compiler", "build_type", "arch"

    _patch_v2p6p1 = "patches/protobuf_v2.6.1.patch"
    exports_sources = [
        _patch_v2p6p1,
        "CMakeLists.txt",
    ]
    tool_requires = [
        "autoconf/2.71",
        "libtool/2.4.7",
    ]

    _src_dir = "protobuf"
    _git_url = url
    _git_ref = f"v{version}"
    _git_dir = "protobuf"

    def source(self):
        git = scm.Git(self, self._git_dir)
        git.clone(self._git_url, args=["depth=1", "--branch", self._git_ref])

    def generate(self):
        tc = gnu.AutotoolsToolchain(self)
        tc.generate()
        # files.patch(self, base_path=self._src_dir, patch_file=self._patch_v2p6p1)

    def build(self):
        autotools = gnu.Autotools(self)
        autotools.configure()
        autotools.make()

    def package(self):
        autotools = gnu.Autotools(self)
        autotools.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_module_file_name", "Protobuf")
        self.cpp_info.set_property("cmake_file_name", "protobuf")
        self.cpp_info.set_property("pkg_config_name", "protobuf_full_package")

        self.cpp_info.libs = tools.collect_libs(self)
