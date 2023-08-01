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
Implementation for cloe-launch command shell.

Usage: cloe-launch [-v] shell [-c] [-e VAR] [-E] CONANFILE [CONAN_INSTALL_ARGS] -- [SHELL_ARGS]
"""

from typing import List

import click

from cloe_launch import Configuration
from cloe_launch.exec import Engine
from . import options


@click.command("shell")
@options.preserve_env()
@options.override_env()
@options.cache()
@options.conanfile()
@options.args()
@click.pass_obj
def cli_shell(
    conf: Configuration,
    preserve_env: bool,
    override_env: List[str],
    cache: bool,
    conanfile: str,
    args: List[str],
):
    """Launch shell with the correct environment from a profile."""
    engine = Engine(conf, conanfile=conanfile)
    engine.preserve_env = preserve_env
    engine.conan_args = options.extract_conan_args(args)

    # Replace process with shell.
    engine.shell(options.extract_target_args(args), use_cache=cache)
