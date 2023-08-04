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
Implementation for cloe-launch command deploy.

Usage: cloe-launch [-v] deploy [-c] [-f] [-r] [-D PATH] CONANFILE [CONAN_INSTALL_ARGS]
"""

import sys
import pathlib
from typing import List

import click

from cloe_launch import Configuration
from cloe_launch.exec import Engine
from . import _options
from ._options import cli_command


@cli_command("deploy")
@click.option(
    "-D",
    "--dest",
    type=click.Path(file_okay=False, dir_okay=True),
    help="Destination directory, for example /usr/local.",
    prompt="Destination directory",
)
@click.option(
    "-f",
    "--force",
    is_flag=True,
    help="Overwrite existing files.",
)
@click.option(
    "--rpath/--no-rpath",
    is_flag=True,
    default=True,
    help="Set the RPATH of all binaries and libraries.",
)
@_options.conanfile()
@_options.args()
@click.pass_obj
def cli_deploy(
    conf: Configuration,
    dest: str,
    force: bool,
    rpath: bool,
    conanfile: str,
    args: List[str],
):
    """Deploy environment for selected profile.

    This may involve downloading missing and available packages and building
    outdated packages.

    Usage Examples:

    \b
        cloe-launch deploy -f -D /usr/local tests/conanfile.py
        cloe-launch deploy -D deploy tests/conanfile.py
    """
    engine = Engine(conf, conanfile=conanfile)
    engine.conan_args = _options.extract_conan_args(args)

    try:
        engine.deploy(
            pathlib.Path(dest),
            patch_rpath=rpath,
        )
    except ChildProcessError:
        # Most likely scenario:
        # 1. conan had an error and terminated with non-zero error
        # 2. error has already been logged
        sys.exit(1)
