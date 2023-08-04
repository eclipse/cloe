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
Implementation for cloe-launch command activate.
"""

from typing import List

import click

from cloe_launch import Configuration
from cloe_launch.exec import Engine
from . import _options
from ._options import cli_command

@cli_command("activate")
@_options.cache()
@_options.conanfile()
@_options.args()
@click.pass_obj
def cli_activate(
    conf: Configuration,
    cache: bool,
    conanfile: str,
    args: List[str],
):
    """Launch shell with the correct environment from a profile.

    You can then source or evaluate these commands to activate the
    environment:

    \b
    1. source <(cloe-launch activate [options])
    2. eval $(cloe-launch activate [options])

    If you plan on putting this in your shell, it is /strongly/ recommended
    to copy the output into your shell or put it in an intermediate file
    instead of calling cloe-launch directly at every new shell invocation!

    \b
    3. cloe-launch activate > ~/.config/cloe/launcher/activate.sh
       echo "source ~/.config/cloe/launcher/activate.sh" >> ~/.bashrc

    \b
    Warnings:
    - If you use method #3 and delete ~/.cache/cloe, you will get errors
      until you re-create the runtime environment.
    - Deleting or overwriting packages in your Conan cache that are used
      in an activated environment is undefined behavior: it can lead to
      unexpected problems!
    - Using cloe shell in combination with cloe activate is currently
      undefined behavior: it can lead to unexpected problems.

    If you want permanence, it's probably better to use the `deploy`
    command instead of `activate`.
    """
    engine = Engine(conf, conanfile=conanfile)
    engine.conan_args = _options.extract_conan_args(args)
    engine.activate(use_cache=cache)
