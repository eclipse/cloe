# Copyright 2024 Robert Bosch GmbH
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
Implementation for cloe-launch command config.

Usage: cloe-launch config
"""

import click

from cloe_launch import Configuration
from ._options import cli_command


@cli_command("config")
@click.option("-e", "--edit", is_flag=True, help="Edit configuration file.")
@click.option("-w", "--write", is_flag=True, help="Write current configuration file.")
@click.pass_obj
def cli_config(
    conf: Configuration,
    edit: bool,
    write: bool,
):
    """Manage launcher configuration.

    Run without any options, the current configuration will be printed to
    stdout. (This is the effective configuration, not the contents of the
    configuration file, which may be less.)

    When using the --edit flag, the EDITOR environment variable is used
    to select the editor to use for the file:

        EDITOR="code -w" cloe-launch config -e

    The configuration file is located at:

        ~/.config/cloe/launcher/conf.toml

    The runtime directory is located at:

        ~/.cache/cloe/launcher/

    Usage Examples:

    \b
        cloe-launch config
        cloe-launch config -we
    """

    if not (write or edit):
        conf.print()
    if write:
        conf.write()
    if edit:
        conf.edit(create = True)
