# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile


class CloeTest(ConanFile):
    python_requires = "cloe-launch-profile/[>=0.20.0]@cloe/develop"
    python_requires_extend = "cloe-launch-profile.Base"

    options = {
        "with_vtd": [True, False]
    }
    default_options = {
        "with_vtd": False
    }

    @property
    def cloe_launch_env(self):
        return {
            "CLOE_ENGINE_WITH_SERVER": "1",
            "CLOE_LOG_LEVEL": "debug",
            "CLOE_STRICT_MODE": "1",
            "CLOE_WRITE_OUTPUT": "0",
            "CLOE_ROOT": Path(self.recipe_folder) / "..",
        }

    def set_version(self):
        self.version = self.project_version("../VERSION")

    def requirements(self):
        self.requires(f"cloe-meta/{self.version}@cloe/develop")
        if self.options.with_vtd:
            self.requires(f"cloe-plugin-vtd/{self.version}@cloe/develop")
            # These dependencies aren't pulled in by the "cloe-plugin-vtd" package,
            # because they are not required to build the package.
            # We need them to run the tests though.
            self.requires("osi-sensor/1.0.0-vtd2.2@cloe/stable")
            self.requires("vtd/2.2.0@cloe-restricted/stable")
