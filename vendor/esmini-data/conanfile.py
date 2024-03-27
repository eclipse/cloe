import os

from conan import ConanFile
from conan.tools import files, env

required_conan_version = ">=1.52.0"


class ESMiniData(ConanFile):
    name = "esmini-data"
    version = "2.37.0"
    license = "MPL-2.0"
    url = "https://github.com/esmini/esmini"
    description = "Basic OpenScenario player example data"
    topics = ("Environment Simulator", "OpenScenario", "OpenDrive")

    def export_sources(self):
        files.export_conandata_patches(self)

    def source(self):
        files.get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def build(self):
        files.apply_conandata_patches(self)

    def package(self):
        files.copy(
            self,
            "*",
            src=os.path.join(self.source_folder, "resources"),
            dst=os.path.join(self.package_folder),
        )
        files.copy(
            self,
            "*.xosc",
            src=os.path.join(self.source_folder, "EnvironmentSimulator", "code-examples", "test-driver"),
            dst=os.path.join(self.package_folder, "xosc"),
        )

    def package_info(self):
        self.runenv_info.define_path(
            "ESMINI_XOSC_PATH", os.path.join(self.package_folder, "xosc")
        )
        self.runenv_info.define_path(
            "ESMINI_XODR_PATH", os.path.join(self.package_folder, "xodr")
        )
