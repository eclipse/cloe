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
Common options used by cloe-launch commands.
"""

import os

from typing import Tuple, List, Dict

import click


def conanfile():
    """Click argument CONANFILE."""
    return click.argument(
        "conanfile", type=click.Path(exists=True, file_okay=True, dir_okay=False)
    )


def args():
    """Click argument ARGS."""
    return click.argument("args", nargs=-1)


def split_args(xargs) -> Tuple[List[str], List[str]]:
    """Split xargs as (CONAN_INSTALL_ARGS -- TARGET_ARGS)."""
    conan_args = []
    target_args = []
    right_of_sep = False
    for item in xargs:
        if right_of_sep:
            target_args.append(item)
        elif item == "--":
            right_of_sep = True
        else:
            conan_args.append(item)
    return (conan_args, target_args)


def extract_conan_args(xargs) -> List[str]:
    """Return CONAN_INSTALL_ARGS from xargs."""
    results = []
    for item in xargs:
        if item == "--":
            break
        results.append(item)
    return results


def extract_target_args(xargs) -> List[str]:
    """Return TARGET_ARGS from xargs."""
    results = []
    should_append = False
    for item in xargs:
        if should_append:
            results.append(item)
        elif item == "--":
            should_append = True
    return results


def preserve_env():
    """Click option --preserve-env."""
    return click.option(
        "-E",
        "--preserve-env",
        is_flag=True,
        help="Preserve user environment.",
    )


def override_env():
    """Click option --override-env."""
    return click.option(
        "-e",
        "--override-env",
        multiple=True,
        type=click.STRING,
        help="Use environment variable as set or preserve.",
    )


def process_overrides(overrides: List[str]) -> Dict[str, str]:
    """Convert KEY=VALUE lines to a dictionary."""
    results = {}
    for line in overrides:
        kv = line.split("=", 1)
        if len(kv) == 1:
            kv.append(os.getenv(kv[0], ""))
        results[kv[0]] = kv[1]
    return results


def cache():
    """Click option --cache/--no-cache."""
    return click.option(
        "-c",
        "--cache/--no-cache",
        is_flag=True,
        help="Re-use the cache if available.",
    )
