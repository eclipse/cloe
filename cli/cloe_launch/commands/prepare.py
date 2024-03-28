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
Implementation for cloe-launch command prepare.

Usage: cloe-launch prepare CONANFILE [CONAN_INSTALL_ARGS]
"""

import sys
from typing import List

import click

from cloe_launch import Configuration
from cloe_launch.exec import Engine
from . import _options
from ._options import cli_command


@cli_command("prepare")
@_options.conanfile()
@_options.args()
@click.pass_obj
def cli_prepare(
    conf: Configuration,
    conanfile: str,
    args: List[str],
):
    """Prepare environment for selected profile.

    This involves downloading missing and available packages and building
    outdated packages.

    No options to cloe-launch may appear after specifying the conanfile.
    All options encountered after the conanfile and before -- will be
    passed to `conan install`. See `conan install --help` for help on
    which options are available here.

    Usage Examples:

    \b
        cloe-launch prepare tests/conanfile.py --build=missing
        cloe-launch prepare tests/conanfile.py -o cloe-engine:server=False --build=outdated
        cloe-launch prepare tests/conanfile.py -s build_type=Debug --build="*/*@cloe/develop"
        cloe-launch prepare tests/conanfile.py --require-override boost/1.81
    """
    engine = Engine(conf, conanfile=conanfile)
    engine.conan_args = _options.extract_conan_args(args)

    try:
        engine.prepare()
    except ChildProcessError:
        # Most likely scenario:
        # 1. conan had an error and terminated with non-zero error
        # 2. error has already been logged
        sys.exit(1)
