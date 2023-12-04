# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import files, scm

required_conan_version = ">=1.52.0"


class CloeMeta(ConanFile):
    name = "cloe-meta"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Closed-loop automated driving simulation environment"
    topics = ["simulation"]
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_vtd": [True, False],
        "with_engine": [True, False],
    }
    default_options = {
        "with_vtd": False,
        "with_engine": True,

        "fable:allow_comments": True,
        "engine:server": True,
        "engine:lrdb": True,
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
        cloe_requires("cloe-plugin-basic")
        cloe_requires("cloe-plugin-gndtruth-extractor")
        cloe_requires("cloe-plugin-minimator")
        cloe_requires("cloe-plugin-mocks")
        cloe_requires("cloe-plugin-noisy-sensor")
        cloe_requires("cloe-plugin-speedometer")
        cloe_requires("cloe-plugin-virtue")
        if self.options.with_vtd:
            cloe_requires("cloe-plugin-vtd")

        if self.options.with_engine:
            cloe_requires("cloe-engine")

        # Overrides:
        self.requires("fmt/9.1.0", override=True)
        self.requires("inja/3.4.0", override=True)
        self.requires("nlohmann_json/3.11.2", override=True)
        self.requires("incbin/cci.20211107", override=True),
        self.requires(f"boost/[>=1.65.0]", override=True)
