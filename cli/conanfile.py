# mypy: ignore-errors
# pylint: skip-file

from pathlib import Path

import toml
from conan import ConanFile
from conan.tools import files, scm, env

required_conan_version = ">=1.52.0"

class Base(object):
    settings = "os", "compiler", "build_type", "arch"

    def generate(self):
        if "cloe_launch_env" not in dir(self):
            return

        export = env.Environment()
        for k, v in self.cloe_launch_env.items():
            export.define(k, str(v))
        env_vars = export.vars(self)

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
                return files.load(self, version_file).strip()
        git = scm.Git(self, self.recipe_folder)
        return git.run("describe --dirty=-dirty")[1:]


class CloeLaunchProfile(ConanFile):
    name = "cloe-launch-profile"
    license = "Apache-2.0"
    url = "https://github.com/eclipse/cloe"
    description = "Provides base class for cloe-launch profiles"
    user = "cloe"
    channel = "develop"

    def set_version(self):
        pyproject = toml.loads(files.load(self, "pyproject.toml"))
        self.version = pyproject["project"]["version"]
