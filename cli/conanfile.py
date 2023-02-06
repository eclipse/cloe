# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

from conans import ConanFile, tools
from conan.tools.env import Environment

class Base(object):
    settings = "os", "compiler", "build_type", "arch"
    generators = "virtualenv", "virtualrunenv"

    def generate(self):
        if "cloe_launch_env" not in dir(self):
            return

        env = Environment()
        for k, v in self.cloe_launch_env.items():
            env.define(k, str(v))
        env_vars = env.vars(self)

        # Raw environment variables
        env_file = Path(self.generators_folder) / "environment_cloe_launch.sh.env"
        with open(env_file, "w", encoding="utf-8") as file:
            for (key, value) in env_vars.items():
                value = value.replace('"', '\\"')
                print(f'{key}="{value}"', file=file)

        # Alternative runtime file
        env_vars.save_script("cloe_launch_env")

    def project_version(self, version_path):
        if version_path:
            version_file = Path(self.recipe_folder) / version_path
            if version_file.exists():
                return tools.load(version_file).strip()
        git = tools.Git(folder=self.recipe_folder)
        return git.run("describe --dirty=-dirty")[1:]


class CloeLaunchProfile(ConanFile):
    name = "cloe-launch-profile"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Provides base class for cloe-launch profiles"
    user = "cloe"
    channel = "develop"

    def set_version(self):
        for line in tools.load("setup.py").split("\n"):
            if not line.strip().startswith("version="):
                continue
            self.version = line.strip().split("=")[1].strip('",')
            return
        raise Exception("cannot find cloe-launch version")
