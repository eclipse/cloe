import os
from pathlib import Path
from conans import ConanFile, tools


class CloeTest(ConanFile):
    python_requires = "cloe-launch-profile/[~=0.19.0]@cloe/develop"
    python_requires_extend = "cloe-launch-profile.Base"

    default_options = {
        "cloe-engine:server": True,
    }

    @property
    def cloe_launch_env(self):
        return {
            "CLOE_LOG_LEVEL": "debug",
            "CLOE_STRICT_MODE": "1",
            "CLOE_WRITE_OUTPUT": "0",
        }

    def set_version(self):
        for version_path in ["../VERSION", "../../../VERSION"]:
            version_file = Path(self.recipe_folder) / version_path
            if version_file.exists():
                self.version = self.project_version(version_path)
                return
        git = tools.Git(folder=self.recipe_folder)
        self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-engine/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-basic/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-noisy-sensor/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-speedometer/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-virtue/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-vtd/{self.version}@cloe/develop")

        # Runtime requirements for VTD.
        self.requires("osi-sensor/1.0.0-vtd2.2@cloe/stable")
        self.requires("vtd/2.2.0@cloe-restricted/stable")

        if self.options["cloe-engine"].server:
            self.requires("boost/[<1.70]", override=True)
