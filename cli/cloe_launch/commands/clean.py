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

# pylint: disable=too-many-arguments

"""
Implementation for cloe-launch command clean.

Usage: cloe-launch clean CONANFILE
"""

import os
import shutil

import click

from cloe_launch import Configuration
from cloe_launch.exec import Engine
from . import _options
from .options import cli_command


@cli_command("clean")
@click.option("--all", is_flag=True, help="Clean all runtime environments.")
@_options.conanfile()
@click.pass_obj
def cli_clean(
    conf: Configuration,
    all: bool,
    conanfile: str,
):
    """Clean launcher runtime environment for specified configuration.

    The runtime environment is by default at `~/.cache/cloe/launcher/`
    in a directory named after a hash calculated from the conanfile.
    Currently, the hash does not take additional arguments into account
    (this should change in the future).

    It is safe to clean a configuration or all configurations.
    Almost all commands will regenerate these configurations if needed
    within a few seconds. However, the following cases should be noted:

    \b
    1. If the `deploy` command is used with a configuration,
       an uninstall script is stored in the cache.
       This should be preserved if you want to completely
       remove a deployed configuration at a later time.
    2. If the output of the `activate` command is used,
       for example in the `.bashrc` file,
       then you should not clean the configuration.

    Usage Examples:

    \b
        cloe-launch clean tests/conanfile.py
        cloe-launch clean -a
    """
    engine = Engine(conf, conanfile=conanfile)

    if all:
        if os.path.exists(conf.runtime_dir):
            shutil.rmtree(conf.runtime_dir, ignore_errors=True)
    else:
        engine.clean()
