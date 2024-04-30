# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conan import ConanFile
from conan.tools import cmake, files, scm

required_conan_version = ">=1.52.0"


class Cloe(ConanFile):
    name = "cloe"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Closed-loop automated driving simulation environment"
    topics = ["simulation"]
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_vtd": [True, False],
        "with_esmini": [True, False],
        "with_engine": [True, False],
    }
    default_options = {
        "with_vtd": False,
        "with_esmini": True,
        "with_engine": True,

        "cloe-engine:server": True,
    }
    no_copy_source = True

    def set_version(self):
        version_file = Path(self.recipe_folder) / "VERSION"
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

        boost_version = "1.74.0"
        if self.options.with_engine:
            cloe_requires("cloe-engine")

        # Overrides:
        self.requires("zlib/1.2.13", override=True)
        self.requires("fmt/9.1.0", override=True)
        self.requires("inja/3.4.0", override=True)
        self.requires("nlohmann_json/3.11.2", override=True)
        self.requires("incbin/cci.20211107", override=True),
        self.requires(f"boost/{boost_version}", override=True)
