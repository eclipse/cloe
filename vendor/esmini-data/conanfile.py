import os

from conan import ConanFile
from conan.tools import files, env

required_conan_version = ">=1.52.0"


class ESMiniData(ConanFile):
    name = "esmini-data"
    license = "MPL-2.0"
    url = "https://github.com/esmini/esmini"
    description = "Basic OpenScenario player example data"
    topics = ("Environment Simulator", "OpenScenario", "OpenDrive")

    # This package may not be built while all other packages are built
    # because it is solely a runtime dependency. Because it only copies
    # a few files, the only thing that may take a longer period of time
    # is downloading the esmini archive. For now, this is acceptable.
    build_policy = "missing"

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
