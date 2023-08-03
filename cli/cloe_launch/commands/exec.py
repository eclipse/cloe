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
Implementation for cloe-launch command exec.

Usage: cloe-launch [-v] exec [-c] [-e VAR] [-E] [-d] CONANFILE [CONAN_INSTALL_ARGS] -- [ENGINE_ARGS]
"""

import sys
from typing import List

import click

from cloe_launch import Configuration
from cloe_launch.exec import Engine
from . import options


@click.command("exec")
@options.preserve_env()
@options.override_env()
@options.cache()
@click.option(
    "-d",
    "--debug",
    is_flag=True,
    help="Launch cloe-engine with GDB.",
)
@options.conanfile()
@options.args()
@click.pass_obj
def cli_exec(
    conf: Configuration,
    preserve_env: bool,
    override_env: List[str],
    cache: bool,
    debug: bool,
    conanfile: str,
    args: List[str],
):
    """Launch cloe-engine from a Conan recipe.

    No options to cloe-launch may appear after specifying the conanfile.
    All options encountered after the conanfile and before -- will be
    passed to `conan install`. See `conan install --help` for help on
    which options are available here. All arguments after -- will be
    passed to the `cloe-engine` binary.

    Usage Examples:

    \b
        cloe-launch exec -c tests/conanfile.py -- usage
        cloe-launch exec -c tests/conanfile.py -- -l debug run tests/smoketest.json
    """
    engine = Engine(conf, conanfile=conanfile)
    engine.conan_args = options.extract_conan_args(args)
    engine.preserve_env = preserve_env

    # Run cloe-engine and pass on returncode:
    # If cloe-engine is killed/aborted, subprocess will return 250.
    engine_args = options.extract_target_args(args)
    overrides = options.process_overrides(override_env)
    result = engine.exec(
        engine_args, use_cache=cache, debug=debug, override_env=overrides
    )
    sys.exit(result.returncode)
