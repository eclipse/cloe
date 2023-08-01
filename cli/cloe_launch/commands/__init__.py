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
Implementation for cloe-launch commands.

Export all the click commands into this namespace so we can just do:

    from cloe_launch.commands import *

Then add the command to the main group:

    main.add_command(cli_activate)
"""

import logging
import sys

from typing import Optional

import click

from cloe_launch import Configuration, ConfigurationError
from .activate import cli_activate
from .clean import cli_clean
from .deploy import cli_deploy
from .exec import cli_exec
from .prepare import cli_prepare
from .shell import cli_shell


@click.group(context_settings={"allow_interspersed_args": False})
@click.option(
    "-v",
    "--verbose",
    envvar="CLOE_VERBOSE",
    count=True,
    help="Print more information (mostly debugging).",
)
@click.option(
    "--conan-path",
    envvar="CLOE_CONAN_PATH",
    type=click.Path(exists=True, file_okay=True, dir_okay=False),
    help="Path to conan executable to use.",
)
@click.version_option()
@click.pass_context
def cli(
    ctx: click.Context,
    verbose: int,
    conan_path: Optional[str],
):
    """
    Prepare and use Conan virtual environments for cloe-engine configurations.

    While you can use Conan to do this by hand, cloe-launch will manage these
    for you in `~/.cache/cloe/launcher/`. This lets you do things like:

        \b
        $ cloe-launch prepare tests/conanfile.py
        $ cloe-launch shell tests/conanfile.py
        # cloe-engine run tests/testcase.lua

    The first command will fetch and build any missing requirements;
    the second command will start a new shell environment (with cloe-engine in
    the PATH and any included plugin directories in CLOE_PLUGIN_PATH);
    the third command will run cloe-engine in the temporary environment.
    Type `exit` or press Ctrl+D to exit the shell.

    Note: Under the hood, cloe-launch makes use of `conan install` for each
    command. In the extreme case, it's possible to pass arguments to three to
    four different components:

        \b
        cloe-launch [OPTS] CMD [CMD_OPTS] CONANFILE [CONAN_ARGS] -- [EXTRA_ARGS]
        \b
        where:
            OPTS are options for cloe-launch
            CMD_OPTS are options for cloe-launch subcommand CMD
            CONAN_ARGS are arguments for conan install
            EXTRA_ARGS are arguments for cloe-engine or the shell, depending on CMD

    Option and argument order matters here! All arguments after the first
    positional argument CONANFILE are split by `--` and passed to conan install
    and whatever is run in the Conan virtual environment.

    For example, none of these options can be rearranged:

        \b
        cloe-launch -v exec -c conanfile.py --build="*/*@cloe/develop" -- -l debug run test.lua

    For normal usage, it's recommended to just start a subshell and work
    within it. When you need the extra arguments though, they come in handy.

    For more information, see the help on each command.
    """
    if verbose == 0:
        level = logging.WARNING
    elif verbose == 1:
        level = logging.INFO
    else:
        level = logging.DEBUG
    logging.basicConfig(format="%(message)s", stream=sys.stderr, level=level)

    ctx.obj = Configuration()
    if conan_path:
        ctx.obj._conf["conan_path"] = conan_path


def main():
    """Run the main click command."""

    cli.add_command(cli_activate)
    cli.add_command(cli_clean)
    cli.add_command(cli_exec)
    cli.add_command(cli_deploy)
    cli.add_command(cli_prepare)
    cli.add_command(cli_shell)

    try:
        cli()
    except ConfigurationError as err:
        print(f"Error: {err}", file=sys.stderr)
