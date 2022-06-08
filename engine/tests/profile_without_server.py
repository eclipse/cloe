from pathlib import Path
from conans import ConanFile, tools


class CloeTest(ConanFile):
    name = "cloe-engine-test"
    settings = "os", "compiler", "build_type", "arch"
    default_options = {
        "cloe-engine:server": False,
    }
    generators = "virtualenv", "virtualrunenv"

    def set_version(self):
        version_file = Path(self.recipe_folder) / "../../VERSION"
        if version_file.exists():
            self.version = tools.load(version_file).strip()
        else:
            git = tools.Git(folder=self.recipe_folder)
            self.version = git.run("describe --dirty=-dirty")[1:]

    def requirements(self):
        self.requires(f"cloe-engine/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-basic/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-mocks/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-noisy-sensor/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-speedometer/{self.version}@cloe/develop")
        self.requires(f"cloe-plugin-virtue/{self.version}@cloe/develop")

    def package_info(self):
        self.env_info.CLOE_ENGINE_WITH_SERVER = "1"
        self.env_info.CLOE_LOG_LEVEL = "debug"
        self.env_info.CLOE_STRICT_MODE = "1"
        self.env_info.CLOE_WRITE_OUTPUT = "0"
