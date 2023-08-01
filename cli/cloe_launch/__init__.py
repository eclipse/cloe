"""
This module contains the launcher configuration data types.
"""

import logging
import os
import shutil
import subprocess

from typing import (
    List,
    Optional,
)

import toml


class ConfigurationError(Exception):
    """ConfigurationError signifies a problem with the launcher configuration."""


class Configuration:
    """Configuration contains the launcher configuration as read from file."""

    config_dir = os.path.expanduser("~/.config/cloe/launcher/")

    config_file = os.path.expanduser("~/.config/cloe/launcher/conf.toml")
    runtime_dir = os.path.expanduser("~/.cache/cloe/launcher")

    conf_version = "1"
    _conf = {
        "version": conf_version,
        "shell_path": "/bin/bash",
        "conan_path": "conan",
        "relay_anonymous_files": True,
        "engine": {
            "pre_arguments": [],
            "post_arguments": [],
        },
    }

    def __init__(self):
        # Make configuration and runtime directories if needed:
        if not os.path.exists(self.config_dir):
            logging.info("Create configuration directory: %s", self.config_dir)
            os.makedirs(self.config_dir)
        if not os.path.exists(self.runtime_dir):
            logging.info("Create runtime directory: %s", self.runtime_dir)
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

    def profile_runtime(self, hash: str) -> str:
        """Return the path to the runtime directory of the profile."""
        return os.path.join(self.runtime_dir, hash)

    def write(self) -> None:
        """Write current configuration to the disk."""
        logging.info(f"Write configuration to {self.config_file}:\n  {self._conf}")
        with open(self.config_file, "w", encoding="utf-8") as file:
            toml.dump(self._conf, file)

    def edit(self, create: bool = False) -> None:
        """Open the configuration in the user's editor."""
        editor = os.getenv("EDITOR")
        if editor is None:
            raise ConfigurationError("environment variable EDITOR is unset")
        if not create and not os.path.exists(self.config_file):
            raise ConfigurationError(f"configuration {self.config_file} does not exist")
        cmd = [editor, self.config_file]
        logging.info("Exec: %s", " ".join(cmd))
        subprocess.call(cmd)
