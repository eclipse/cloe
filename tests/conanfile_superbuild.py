# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import files, scm

required_conan_version = ">=1.52.0"


class CloeSuperbuildTest(ConanFile):
    python_requires = "cloe-launch-profile/[>=0.20.0]@cloe/develop"
    python_requires_extend = "cloe-launch-profile.Base"

    name = "cloe-superbuild-test"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Closed-loop automated driving simulation environment"
    topics = ["simulation"]
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_esmini": [True, False],
        "with_vtd": [True, False],
    }
    default_options = {
        "with_esmini": True,
        "with_vtd": False,

        "cloe:fable_allow_comments": True,
        "cloe:engine_server": True,
    }

    @property
    def cloe_launch_env(self):
        return {
            "CLOE_ENGINE_WITH_SERVER": "1" if self.options["cloe"].engine_server else "0",
            "CLOE_LOG_LEVEL": "debug",
            "CLOE_STRICT_MODE": "1",
            "CLOE_WRITE_OUTPUT": "0",
            "CLOE_ROOT": Path(self.recipe_folder) / "..",
        }

    def configure(self):
        self.options["cloe"].with_esmini = self.options.with_esmini
        self.options["cloe"].with_vtd = self.options.with_vtd

    def set_version(self):
        self.version = self.project_version("../VERSION")

    def requirements(self):
        self.requires(f"cloe/{self.version}@cloe/develop")

        # Runtime requirements:
        if self.options.with_esmini:
            self.requires("esmini-data/2.37.4@cloe/stable")
        if self.options.with_vtd:
            self.requires("osi-sensor/1.0.0-vtd2.2@cloe/stable")
            self.requires("vtd/2.2.0@cloe-restricted/stable")
