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

import logging
import os
import re
import select
import shlex
import subprocess
import textwrap

from collections import OrderedDict
from pathlib import Path
from typing import Dict, List, Optional, Mapping, Union

from rich.console import Console
from rich.style import Style
from rich.live import Live
from rich.text import Text


class Environment:
    """This class stores a set of environment variables for runtimes.

    The following important variables are required for correct execution:

        PATH

    The main uses are to allow plugins to specify or modify variables that
    they will need when loaded. For example:

        LD_LIBRARY_PATH
        LD_PRELOAD

    Another use is to specify variables that are required for correct
    interpolation of stackfiles:

        CLOE_ROOT
        VTD_ROOT
        IPGHOME

    As one of the primary use-cases of the Environment class is to allow
    modification of the *PATH variables, we provide an extra method for
    modifying path variables.

    Caveats:
    - This is a global namespace, so it's problematic to combine two
      plugins that require different environment variables.
    - Environment variables that contain ":" separated lists do not take
      escaping into account.
    """

    _sep = os.pathsep
    _shell_path: str = "/bin/bash"
    _shell_env: Dict[str, str] = {
        "PATH": "/bin:/usr/bin",
    }
    _preserve: List[str] = [
        # Required for avoiding recursive invocations:
        "CLOE_SHELL",
        # Required for XDG compliance:
        "XDG_.*",
        "HOME",
        "USER",
        # Preserve locale settings:
        "LANGUAGE",
        "LANG",
        "LC_.*",
        # Required for Zsh
        "ZDOTDIR",
        # Required for resolving relative paths:
        "PWD",
        # Required for graphical output:
        "XAUTHORITY",
        "DISPLAY",
        # Required for correct terminal output:
        "COLORTERM",
        "TERM",
        "TERMINAL",
        "S_COLORS",
        # Required for working with proxies:
        "(HTTP|HTTPS|FTP|SOCKS|NO)_PROXY",
        "(http|https|ftp|socks|no)_proxy",
    ]
    _data: Dict[str, str] = {}

    def __init__(
        self,
        env: Union[Path, List[Path], Dict[str, str], None],
        preserve: Optional[List[str]] = None,
        source_file: bool = True,
    ):
        # Initialize shell environment
        if preserve is not None:
            self._preserve = preserve
        self._init_shell_env(os.environ)

        # Initialize the environment
        if isinstance(env, str):
            env = [Path(env)]
        if isinstance(env, Path):
            env = [env]
        if isinstance(env, list):
            self._data = {}
            if source_file:
                self.init_from_shell(env)
            else:
                for file in env:
                    self.init_from_file(file)
            if len(env) > 1:
                self.deduplicate_list("PATH")
        elif isinstance(env, dict):
            self.init_from_dict(env)
        else:
            self.init_from_env()

    def _init_shell_env(self, base: Mapping[str, str]) -> None:
        preserve = "|".join(self._preserve)
        regex = re.compile(f"^({preserve})$")
        for key in base:
            if regex.match(key):
                self._shell_env[key] = base[key]

    def init_from_file(self, filepath: Path) -> None:
        """Init variables from a file containing KEY=VALUE pairs."""
        with filepath.open() as file:
            data = file.read()
        self.init_from_str(data)

    def init_from_dict(self, env: Dict[str, str]) -> None:
        """Init variables from a dictionary."""
        self._data = env

    def init_from_shell(
        self, filepaths: List[Path], shell: Optional[str] = None
    ) -> None:
        """Init variables from a shell sourcing a file."""
        assert len(filepaths) != 0
        if shell is None:
            shell = self._shell_path
        quoted_filepaths = [shlex.quote(x.as_posix()) for x in filepaths]
        cmd = [
            shell,
            "-c",
            f"source {' && source '.join(quoted_filepaths)} &>/dev/null && env",
        ]
        result = system(cmd, env=self._shell_env)
        if result.returncode != 0:
            logging.error(
                f"Error: error sourcing files from shell: {', '.join(quoted_filepaths)}"
            )
        self.init_from_str(result.stdout)

    def init_from_env(self) -> None:
        """Init variables from this program's environment."""
        self._data = os.environ.copy()

    def init_from_str(self, data: str) -> None:
        """
        Init variables from a string of KEY=VALUE lines.

        - Leading and trailing whitespace is stripped.
        - Quotes are not removed.
        - Empty lines and lines starting with # are ignored.
        """
        for line in data.split("\n"):
            if line.strip() == "":
                continue
            if line.startswith("#"):
                continue

            kv = line.split("=", 1)
            try:
                self._data[kv[0]] = kv[1]
            except IndexError:
                logging.error(
                    f"Error: cannot interpret environment key-value pair: {line}"
                )

    def __delitem__(self, key: str):
        self._data.__delitem__(key)

    def __getitem__(self, key: str) -> str:
        return self._data.__getitem__(key)

    def __setitem__(self, key: str, value: str) -> None:
        self._data.__setitem__(key, value)

    def __str__(self) -> str:
        indent = "    "

        buf = "{\n"
        for k in sorted(self._data.keys()):
            buf += indent + k + ": "
            val = self._data[k]
            if k.endswith("PATH"):
                lst = val.split(self._sep)
                buf += "[\n"
                for path in lst:
                    buf += indent + indent + path + "\n"
                buf += indent + "]"
            else:
                buf += val
            buf += "\n"
        buf += "}"
        return buf

    def path_append(self, key: str, value: Union[Path, str]) -> None:
        """
        Append the value to the path-like variable key.

        This uses ":" as the separator between multiple values in the path.
        """
        if key in self._data:
            self._data[key] += self._sep + str(value)
        else:
            self._data[key] = str(value)

    def path_prepend(self, key: str, value: Union[Path, str]) -> None:
        """
        Prepend the value to the path-like variable key.

        This uses ":" as the separator between multiple values in the path.
        """
        if key in self._data:
            self._data[key] = self._sep + str(value) + self._data[key]
        else:
            self._data[key] = str(value)

    def path_set(self, key: str, values: List[Union[Path, str]]) -> None:
        """Set the key to a :-separated list."""
        self._data[key] = self._sep.join([str(v) for v in values])

    def deduplicate_list(self, key: str) -> None:
        """Remove duplicates from the specified key."""
        if key not in self._data:
            return
        self._data[key] = self._sep.join(
            list(OrderedDict.fromkeys(self._data[key].split(self._sep)))
        )

    def get(self, key: str, default: Optional[str] = None) -> Optional[str]:
        """Get the value at key or return default."""
        return self._data.get(key, default)

    def get_list(
        self, key: str, default: Optional[List[str]] = None
    ) -> Optional[List[str]]:
        """Get the value at key and split or return default.

        Note: The list may contain empty entries, as is the case when a trailing
        colon or sandwiched colon is present:

            PATH=/bin:/usr/bin:
            PATH=:/bin:/usr/bin
            PATH=/bin::/usr/bin

        In each of these these examples, an empty string is present in the list.
        These may be interpreted by the shell and many programs as the current directory.
        This has been experimentally verified with the PATH variable in Zsh and Bash.
        """
        if key in self._data:
            return self._data[key].split(self._sep)
        return default

    def set(self, key: str, value: Optional[Union[Path, str]]) -> None:
        """Set the value."""
        self._data[key] = str(value)

    def set_default(self, key: str, value: str) -> None:
        """Set the value if it has not already been set."""
        if key not in self._data:
            self._data[key] = value

    def has(self, key: str) -> bool:
        """Return true if the key is in the environment."""
        return key in self._data

    def preserve(self, key: str, override: bool = False):
        """
        Set the given key if it's not already set and it's in the environment.

        When override is True, the value is taken from the environment
        regardless of whether it already exists or not.

        This should not be used for path-like variables.
        """
        if key not in self._data or override:
            value = os.getenv(key, None)
            if value is not None:
                self._data[key] = value

    def export(self, filepath: Path) -> None:
        """Write the environment variables to a file in KEY=VALUE pairs."""
        with filepath.open("w") as file:
            for k in self._data.keys():
                qv = shlex.quote(self._data[k])
                file.write(f"{k}={qv}\n")

    def as_dict(self) -> Dict[str, str]:
        """Return a reference to the internal dictionary."""
        return self._data


def system(
    cmd: List[str],
    env: Optional[Dict[str, str]] = None,
    must_succeed: bool = True,
    capture_output: bool = True,
) -> subprocess.CompletedProcess:
    """Run a command quietly, only printing stderr if the command fails."""

    logging.info(f"Exec: {' '.join(cmd)}")
    result = subprocess.run(
        cmd,
        check=False,
        stdout=subprocess.PIPE if capture_output else None,
        stderr=subprocess.STDOUT if capture_output else None,
        universal_newlines=True,
        env=env,
    )
    if result.returncode != 0:
        logging.error(f"Error running: {' '.join(cmd)}")
        if result.stdout is not None:
            logging.error(result.stdout)
        if must_succeed:
            raise ChildProcessError()
    return result


class TransientResult:
    command: List[str]
    env: Optional[Dict[str, str]]
    merged_output: List[str]
    stdout_lines: List[str]
    stderr_lines: List[str]
    return_code: int

    def __init__(self, command: List[str], env: Optional[Dict[str, str]]):
        self.command = command
        self.env = env

def _readlines_noblock(fd):
    lines = []
    while True:
        line = fd.readline()
        if line == '':
            break
        lines.append(line)
    return lines


def transient_system(
    command: List[str],
    environment: Optional[Dict[str, str]] = None,
    capture_output: bool = True,
    transient_title: Optional[str] = None,
    transient_lines: int = 10,
    transient_indent: str = "",
) -> TransientResult:
    """Run a command with transient output, clearing the output when the command is finished."""
    result = TransientResult(command, environment)

    result.merged_output = []
    result.stdout_lines = []
    result.stderr_lines = []
    console = Console(highlight=False, markup=False)
    with Live(console=console, refresh_per_second=10, transient=True) as live:
        last_lines: List[str] = [transient_title] if transient_title else []
        if capture_output:
            live.update(Text("\n".join(last_lines), style="dim"))

        def update_output(line):
            if capture_output:
                if len(last_lines) >= transient_lines:
                    # Remove the oldest line
                    last_lines.pop(1 if transient_title else 0)
                last_lines.append(
                    transient_indent
                    + textwrap.shorten(
                        line.strip(),
                        width=(live.console.width - len(transient_indent)),
                        placeholder="...",
                    )
                )

                # Clear previous lines and print the last 10 lines
                live.update(Text("\n".join(last_lines), style="dim"))
            else:
                live.console.print(line, style="dim", end='')

        with subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            env=environment,
            bufsize=1,
        ) as process:
            assert process.stdout is not None
            assert process.stderr is not None

            poll = select.poll()
            poll.register(process.stdout, select.POLLIN | select.POLLHUP)
            poll.register(process.stderr, select.POLLIN | select.POLLHUP)
            os.set_blocking(process.stdout.fileno(), False)
            os.set_blocking(process.stderr.fileno(), False)
            alive = 2

            events = poll.poll()
            while alive > 0 and len(events) > 0:
                for fd, event in events:
                    if event & select.POLLIN:
                        if fd == process.stdout.fileno():
                            lines = _readlines_noblock(process.stdout)
                            result.stdout_lines.extend(lines)
                        if fd == process.stderr.fileno():
                            lines = _readlines_noblock(process.stderr)
                            result.stderr_lines.extend(lines)
                        result.merged_output.extend(lines)
                        for line in lines:
                            update_output(line)
                    if event & select.POLLHUP:
                        poll.unregister(fd)
                        alive = alive - 1
                    if alive > 0:
                        events = poll.poll()

            result.return_code = process.wait()

    return result
