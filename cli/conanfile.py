from conans import ConanFile, tools
from conan.tools.env import Environment
from pathlib import Path

class Base(object):
    settings = "os", "compiler", "build_type", "arch"
    generators = "virtualenv", "virtualrunenv"

    def generate(self):
        if "cloe_launch_env" not in dir(self):
            return

        env = Environment()
        for k, v in self.cloe_launch_env.items():
            env.define(k, str(v))
        env.vars(self).save_script("cloe_launch_env")

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
