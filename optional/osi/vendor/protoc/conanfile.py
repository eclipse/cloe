from conans import CMake, ConanFile, RunEnvironment, tools
from conans.errors import ConanInvalidConfiguration
from pathlib import Path


class Protoc(ConanFile):
    name = "protoc"
    version = "3.15.5"
    license = "proprietary"
    url = "https://github.com/protocolbuffers/protobuf/releases"
    description = "Google Protocol Buffers Compiler"
    topics = ("serialization", "protobuf")
    settings = {"os": ["Linux"], "arch": ["x86_64"]}

    no_copy_source = True
    build_policy = "missing"

    _filename = f"protoc-{version}-linux-x86_64.zip"

    def source(self):
        tools.download(
            f"{self.url}/download/v{self.version}/{self._filename}", self._filename
        )

    def build(self):
        tools.unzip(
            f"{self.source_folder}/{self._filename}",
            self.build_folder,
            keep_permissions=True,
        )

    def package(self):
        self.copy("*", src="bin", dst="bin")
        self.copy("*", src="include", dst="include")
