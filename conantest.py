from pathlib import Path
from conans import ConanFile, tools


class CloeTest(ConanFile):
    name = "cloe-test"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_vtd": [True, False],
    }
    default_options = {
        "with_vtd": False,
    }
    no_copy_source = True

    def set_version(self):
        version_file = Path(self.recipe_folder) / "VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe/{self.version}@cloe/develop")
        if self.options.with_vtd:
            self.requires("osi-sensor/1.0.0-vtd2.2@cloe/stable")
            self.requires("vtd/2.2.0@cloe-restricted/stable")

    def configure(self):
        self.options["cloe"].with_vtd = self.options.with_vtd
