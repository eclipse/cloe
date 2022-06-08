from pathlib import Path
from conans import ConanFile, tools


class CloeTest(ConanFile):
    name = "cloe-test"
    settings = "os", "compiler", "build_type", "arch"
    generators = "virtualenv", "virtualrunenv"
    default_options = {
        "cloe:with_vtd": False,
        "cloe-engine:server": True,
    }

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe/{self.version}@cloe/develop")
        if self.options["cloe"].with_vtd:
            # These dependencies aren't pulled in by the "cloe" package,
            # because they are not required to build the package.
            # We need them to run the tests though.
            self.requires("osi-sensor/1.0.0-vtd2.2@cloe/stable")
            self.requires("vtd/2.2.0@cloe-restricted/stable")
