# mypy: ignore-errors
# pylint: skip-file
#
# NOTE:
#   This filename needs to not match ^conanfile_*.py$ in order to not be
#   used in the automatic smoketest test suite.

from pathlib import Path

from conan import ConanFile
from conan.tools import files, scm

required_conan_version = ">=1.52.0"


class CloeSuperbuildWithExternalTest(ConanFile):
    """
    This recipe shows how to combine the cloe package -- which provides
    cloe-runtime, etc. -- with packages that depend on these provided
    packages.

    NOTE:
      This test is disabled until the use-case works with Conan.
      See "Known Issues" in the README.md of the repository.
    """

    python_requires = "cloe-launch-profile/[>=0.20.0]@cloe/develop"
    python_requires_extend = "cloe-launch-profile.Base"

    name = "cloe-superbuild-test"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Closed-loop automated driving simulation environment"
    topics = ["simulation"]
    settings = "os", "compiler", "build_type", "arch"

    @property
    def cloe_launch_env(self):
        return {
            "CLOE_ENGINE_WITH_SERVER": "1" if self.options["cloe"].engine_server else "0",
            "CLOE_LOG_LEVEL": "debug",
            "CLOE_STRICT_MODE": "1",
            "CLOE_WRITE_OUTPUT": "0",
            "CLOE_ROOT": Path(self.recipe_folder) / "..",
        }

    def set_version(self):
        self.version = self.project_version("../VERSION")

    def requirements(self):
        self.requires(f"cloe/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-example-external/{self.version}@cloe/develop")
        self.requires("esmini-data/2.37.4@cloe/stable")
        self.requires("fmt/9.1.0", override=True)
