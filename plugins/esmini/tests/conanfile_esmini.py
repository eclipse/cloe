# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path
from conan import ConanFile


class CloeTest(ConanFile):
    python_requires = "cloe-launch-profile/[>=0.20.0]@cloe/develop"
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
            "CLOE_ROOT": Path(self.recipe_folder) / "../../..",
        }

    def set_version(self):
        self.version = self.project_version("../../../VERSION")

    def requirements(self):
        self.requires(f"cloe-engine/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-basic/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-noisy-sensor/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-speedometer/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-gndtruth-extractor/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-virtue/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-esmini/{self.version}@cloe/develop")
        self.requires("esmini-data/2.37.4@cloe/stable")

        # Overrides:
        self.requires("zlib/1.2.13", override=True)
