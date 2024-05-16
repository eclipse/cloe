# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import files, scm

required_conan_version = ">=1.52.0"


class CloeStandardDeployment(ConanFile):
    python_requires = "cloe-launch-profile/[>=0.20.0]@cloe/develop"
    python_requires_extend = "cloe-launch-profile.Base"

    name = "cloe-deployment"
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

        "fable:allow_comments": True,
        "cloe-engine:server": True,
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
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = files.load(self, version_file).strip()
        else:
            git = scm.Git(self, self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        def cloe_requires(dep):
            self.requires(f"{dep}/{self.version}@cloe/develop")

        cloe_requires("cloe-runtime")
        cloe_requires("cloe-models")
        cloe_requires("cloe-osi")
        cloe_requires("cloe-plugin-basic")
        cloe_requires("cloe-plugin-clothoid-fit")
        cloe_requires("cloe-plugin-frustum-culling")
        cloe_requires("cloe-plugin-gndtruth-extractor")
        cloe_requires("cloe-plugin-minimator")
        cloe_requires("cloe-plugin-mocks")
        cloe_requires("cloe-plugin-noisy-sensor")
        cloe_requires("cloe-plugin-speedometer")
        cloe_requires("cloe-plugin-virtue")

        if self.options.with_esmini:
            cloe_requires("cloe-plugin-esmini")

        if self.options.with_vtd:
            cloe_requires("cloe-plugin-vtd")

        # Overrides:
        self.requires("fmt/9.1.0", override=True)
        self.requires("inja/3.4.0", override=True)
        self.requires("nlohmann_json/3.11.3", override=True)
        self.requires("incbin/cci.20211107", override=True)
        self.requires("boost/1.74.0", override=True)
        self.requires("zlib/1.2.13", override=True)
        self.requires("protobuf/3.21.12", override=True)

        # Runtime requirements:
        cloe_requires("cloe-engine")
        if self.options.with_esmini:
            self.requires("esmini-data/2.37.4@cloe/stable")
        if self.options.with_vtd:
            self.requires("osi-sensor/1.0.0-vtd2.2@cloe/stable")
            self.requires("vtd/2.2.0@cloe-restricted/stable")
