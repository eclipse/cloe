# Copyright 2023 Robert Bosch GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

"""
This module contains the launcher configuration data types.
"""

import logging
import os
import shutil
import subprocess
import sys

from pathlib import Path

from typing import (
    List,
    Optional,
)

import toml


class ConfigurationError(Exception):
    """ConfigurationError signifies a problem with the launcher configuration."""


class Configuration:
    """Configuration contains the launcher configuration as read from file."""

    config_dir = Path("~/.config/cloe/launcher/").expanduser()
    config_file = Path("~/.config/cloe/launcher/conf.toml").expanduser()
    runtime_dir = Path("~/.cache/cloe/launcher").expanduser()

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
        if not self.config_dir.exists():
            logging.info("Create configuration directory: %s", self.config_dir)
            self.config_dir.mkdir(parents=True)
        if not self.runtime_dir.exists():
            logging.info("Create runtime directory: %s", self.runtime_dir)
            self.runtime_dir.mkdir(parents=True)

        # Load configuration file:
        if self.config_file.exists():
            if not self.config_file.is_file():
                raise ConfigurationError("configuration file not readable")
            conf = toml.load(self.config_file)
            if "version" not in conf:
                raise ConfigurationError(
                    "missing required version key in configuration"
                )
            for k in conf.keys():
                self._conf[k] = conf[k]

    def print(self):
        toml.dump(self._conf, sys.stdout)

    def profile_runtime(self, hash: str) -> Path:
        """Return the path to the runtime directory of the profile."""
        return self.runtime_dir / hash

    def write(self) -> None:
        """Write current configuration to the disk."""
        logging.info(f"Write configuration to {self.config_file}:\n  {self._conf}")
        with open(self.config_file, "w", encoding="utf-8") as file:
            toml.dump(self._conf, file)

    def edit(self, create: bool = False) -> int:
        """Open the configuration in the user's editor."""
        editor = os.getenv("EDITOR")
        if editor is None:
            raise ConfigurationError("environment variable EDITOR is unset")
        if not create and not self.config_file.exists():
            raise ConfigurationError(f"configuration {self.config_file} does not exist")
        cmd = editor.split(' ') + [str(self.config_file)]
        logging.info("Exec: %s", " ".join(cmd))
        return subprocess.call(cmd)
