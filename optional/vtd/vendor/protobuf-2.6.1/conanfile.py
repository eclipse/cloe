# mypy: ignore-errors
# pylint: skip-file

import os
import subprocess

from conan import ConanFile
from conan.tools import files, gnu

required_conan_version = ">=1.52.0"


class ProtobufConan(ConanFile):
    name = "protobuf"
    version = "2.6.1"
    license = "BSD-3-Clause"
    url = "https://github.com/protocolbuffers/protobuf.git"
    description = "Google Protocol Buffers."
    topics = ("protobuf", "serialization")
    settings = "os", "compiler", "build_type", "arch"
    generators = "AutotoolsToolchain"
    options = {
        "shared": [True, False],
        "lite": [True, False],
    }
    default_options = {
        "shared": True,
        "lite": False,
    }
    exports_sources = [
        "patches/*",
        "cmake/*",
    ]
    build_requires = [
        "autoconf/[>=2.69]",
        "libtool/[>=2.4.6]",
    ]

    @property
    def _is_clang_x86(self):
        return self.settings.compiler == "clang" and self.settings.arch == "x86"

    def configure(self):
        self.options.rm_safe("shared")

    def source(self):
        files.get(self, **self.conan_data["sources"][self.version], strip_root=True)
        for patch in self.conan_data["patches"][self.version]:
            files.patch(self, patch_file=patch)

    def build(self):
        autotools = gnu.Autotools(self)
        if self.should_configure:
            autotools.configure()
        if self.should_build:
            autotools.make()

    @property
    def _cmake_install_base_path(self):
        return os.path.join("lib", "cmake", "protobuf")

    def package(self):
        if not self.should_install:
            return
        autotools = gnu.Autotools(self)
        autotools.install()

        # files.copy(self, "*.proto",
        files.copy(self, "cmake/*", dst=os.path.join(self.package_folder, self._cmake_install_base_path), src=self.source_folder, keep_path=False)

        if not self.options.lite:
            files.rm(self, "libprotobuf-lite*", os.path.join(self.package_folder, "lib"))
            files.rm(self, "libprotobuf-lite*", os.path.join(self.package_folder, "bin"))

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_module_file_name", "Protobuf")
        self.cpp_info.set_property("cmake_file_name", "protobuf")
        self.cpp_info.set_property("pkg_config_name", "protobuf_full_package") # unofficial, but required to avoid side effects (libprotobuf component "steals" the default global pkg_config name)

        build_modules = [
            os.path.join(self._cmake_install_base_path, "protobuf-generate.cmake"),
            os.path.join(self._cmake_install_base_path, "protobuf-module.cmake"),
            os.path.join(self._cmake_install_base_path, "protobuf-options.cmake"),
        ]
        self.cpp_info.set_property("cmake_build_modules", build_modules)

        # libprotobuf
        self.cpp_info.components["libprotobuf"].set_property("cmake_target_name", "protobuf::libprotobuf")
        self.cpp_info.components["libprotobuf"].set_property("pkg_config_name", "protobuf")
        self.cpp_info.components["libprotobuf"].includedirs = ["include"]
        self.cpp_info.components["libprotobuf"].libs = ["protobuf"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["libprotobuf"].system_libs.extend(["m", "pthread"])
            if self._is_clang_x86 or "arm" in str(self.settings.arch):
                self.cpp_info.components["libprotobuf"].system_libs.append("atomic")

        # libprotoc
        self.cpp_info.components["libprotoc"].set_property("cmake_target_name", "protobuf::libprotoc")
        self.cpp_info.components["libprotoc"].libs = ["protoc"]
        self.cpp_info.components["libprotoc"].requires = ["libprotobuf"]

        # libprotobuf-lite
        if self.options.lite:
            self.cpp_info.components["libprotobuf-lite"].set_property("cmake_target_name", "protobuf::libprotobuf-lite")
            self.cpp_info.components["libprotobuf-lite"].set_property("pkg_config_name", "protobuf-lite")
            self.cpp_info.components["libprotobuf-lite"].includedirs = ["include"]
            self.cpp_info.components["libprotobuf-lite"].libs = ["protobuf-lite"]
            if self.settings.os in ["Linux", "FreeBSD"]:
                self.cpp_info.components["libprotobuf-lite"].system_libs.extend(["m", "pthread"])
                if self._is_clang_x86 or "arm" in str(self.settings.arch):
                    self.cpp_info.components["libprotobuf-lite"].system_libs.append("atomic")

        # TODO: Remove once everything is on Conan v2:
        self.cpp_info.filenames["cmake_find_package"] = "Protobuf"
        self.cpp_info.filenames["cmake_find_package_multi"] = "protobuf"
        self.cpp_info.names["pkg_config"] ="protobuf_full_package"
        self.env_info.LD_LIBRARY_PATH.append(os.path.join(self.package_folder, "lib"))
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
