"""
This module contains the launcher configuration data types.
"""

from typing import List, Optional
import os
import shutil
import subprocess
import toml


class ConfigurationError(Exception):
    """ConfigurationError signifies a problem with the launcher configuration."""


class Configuration:
    """Configuration contains the launcher configuration as read from file."""

    config_dir = os.path.expanduser("~/.config/cloe/launcher/")

    config_file = os.path.expanduser("~/.config/cloe/launcher/conf.toml")
    profiles_dir = os.path.join(config_dir, "profiles")
    runtime_dir = os.path.expanduser("~/.cache/cloe/launcher")

    verbose = 0
    conf_version = "1"
    _conf = {
        "version": conf_version,
        "shell_path": "/bin/bash",
        "conan_path": "conan",
        "relay_anonymous_files": True,
        "engine": {"pre_arguments": [], "post_arguments": [],},
    }

    all_profiles: List[str] = []
    default_profile: Optional[str] = None
    current_profile = None

    def __init__(self, profile: str = None, verbose: int = 0):
        self.verbose = verbose

        # Make configuration and runtime directories if needed:
        if not os.path.exists(self.config_dir):
            if self.verbose > 0:
                print("Create configuration directory:", self.config_dir)
            os.makedirs(self.config_dir)
        if not os.path.exists(self.profiles_dir):
            if self.verbose > 0:
                print("Create profile directory:", self.profiles_dir)
            os.makedirs(self.profiles_dir)
        if not os.path.exists(self.runtime_dir):
            if self.verbose > 0:
                print("Create runtime directory:", self.runtime_dir)
            os.makedirs(self.runtime_dir)

        # Load configuration file:
        if os.path.exists(self.config_file):
            conf = toml.load(self.config_file)
            if "version" not in conf:
                raise ConfigurationError(
                    "missing required version key in configuration"
                )
            for k in conf.keys():
                self._conf[k] = conf[k]
            self.default_profile = self._conf["default_profile"]

        # Read all profile names from profile directory
        self.all_profiles = [
            f
            for f in os.listdir(self.profiles_dir)
            if os.path.isfile(self.profile_path(f))
        ]

        # Set current profile
        if profile is not None:
            self.set_current(profile)
        elif self.default_profile is not None:
            self.set_current(self.default_profile)

    def profile_path(self, profile: str) -> str:
        """Return the path to named profile."""
        return os.path.join(self.profiles_dir, profile)

    def profile_runtime(self, profile: str) -> str:
        """Return the path to the runtime directory of the profile."""
        return os.path.join(self.runtime_dir, profile)

    def set_current(self, profile: str) -> None:
        """Set the current profile and make sure it exists."""
        self.current_profile = profile

    def set_default(self, profile: str) -> None:
        """Set the default profile and write it to the configuration."""
        if profile is not None and profile not in self.all_profiles:
            raise ConfigurationError("profile {} does not exist".format(profile))
        self.default_profile = profile
        self._conf["default_profile"] = profile
        if self.verbose > 0:
            print(
                "Write configuration to {}:\n  {}".format(self.config_file, self._conf)
            )
        with open(self.config_file, "w") as file:
            toml.dump(self._conf, file)

    def read(self, profile: str) -> str:
        """Read the specified profile."""
        if self.verbose > 0:
            print("Open:", self.profile_path(profile))
        with open(self.profile_path(profile), "r") as file:
            return file.read()

    def edit(self, profile: str, create: bool = False) -> None:
        """Open the specified profile in the user's editor."""
        editor = os.getenv("EDITOR")
        if editor is None:
            raise ConfigurationError("environment variable EDITOR is unset")
        if not create and not os.path.exists(self.profile_path(profile)):
            raise ConfigurationError("profile {} does not exist".format(profile))
        cmd = [editor, self.profile_path(profile)]
        if self.verbose > 0:
            print("Exec:", " ".join(cmd))
        subprocess.call(cmd)

    def add(self, profile: str, file: str, force: bool = False) -> None:
        """Add the file as a profile."""
        if profile in self.all_profiles and not force:
            raise ConfigurationError(
                "cannot overwrite profile {} unless forced".format(profile)
            )
        if self.verbose > 1:
            print("Copy: {} -> {}".format(file, self.profile_path(profile)))
        shutil.copyfile(file, self.profile_path(profile))
        if profile not in self.all_profiles:
            self.all_profiles.append(profile)

    def remove(self, profile: str) -> None:
        """Remove the profile, if it exists."""
        file = os.path.join(self.profiles_dir, profile)
        if os.path.exists(file):
            if self.verbose > 0:
                print("Remove:", file)
            os.remove(file)
        if self.default_profile == profile:
            self.set_default(None)
