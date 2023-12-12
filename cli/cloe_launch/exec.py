# pylint: disable=logging-fstring-interpolation,invalid-name,too-many-instance-attributes

"""
This module contains classes related to the execution of the engine.

This module also contains the PluginSetup class, which encapsulates the
PluginSetup that each Cloe plugin should implement.
"""

import hashlib
import importlib.util
import logging
import os
import json
import re
import shutil
import subprocess
import sys
import shlex
import platform
import textwrap

from pathlib import Path
from typing import Dict
from typing import Set
from typing import List
from typing import Optional
from typing import Type

from cloe_launch import Configuration, procutils, binutils
from cloe_launch.procutils import Environment


class PluginSetup:
    """
    This class loads a file as a Python module and creates from this module
    the PluginSetup class as an embedded type of this class.
    In other words, this class wraps a file as a PluginSetup.

    Requirements:
    - The file must have the '.py' ending.
    - The file must contain a class named 'PluginSetup'.
    - The class PluginSetup must contain the methods:
        extend_env
        setup
        teardown

    In the future, this should be part of the cloe library such that the
    module can base on it in the same way that Conan files base on
    Conan classes. This allows for better type-checking.
    """

    name = None
    plugin = None

    def __init__(self, env: Environment):
        self.env = env

    def environment(self) -> None:
        """
        Modify environment of cloe-engine.

        This can be used to provide values for interpolation in a stackfile
        as well as values required for successful loading and execution of
        the plugin.

        See the Environment class for more details.
        """

    def setup(self) -> None:
        """Perform plugin setup required for simulation."""

    def teardown(self) -> None:
        """Perform plugin teardown after simulation."""


def _find_plugin_setups(file: Path) -> List[Type[PluginSetup]]:
    """Open a Python module and find all PluginSetups."""
    name = os.path.splitext(file)[0]
    spec = importlib.util.spec_from_file_location(name, file)
    if spec is None:
        return []
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    # Find all PluginSetup sub-classes. This requires PluginSetup to be
    # imported into the namespace.
    return mod.PluginSetup.__subclasses__()


class Engine:
    """Engine represents the configuration of a single execution of cloe-engine."""

    anonymous_file_regex = "^(/proc/self|/dev)/fd/[0-9]+$"
    engine_path = "cloe-engine"

    def __init__(self, conf: Configuration, conanfile: Optional[str] = None):
        # Set options:
        self.conan_path = Path(conf._conf["conan_path"])
        self.shell_path = Path(conf._conf["shell_path"])
        self.relay_anonymous_files = conf._conf["relay_anonymous_files"]
        if conanfile is None:
            self._read_conf_profile(conf)
        else:
            self._read_anonymous_profile(conanfile)
        self.runtime_dir = Path(conf.profile_runtime(self.profile))
        self.engine_pre_args = conf._conf["engine"]["pre_arguments"]
        self.engine_post_args = conf._conf["engine"]["post_arguments"]
        self.abort_recursive_shell: bool = True
        self.preserve_env: bool = False
        self.conan_args: List[str] = []
        self.capture_output = True

        logging.info(f"Profile name: {self.profile}")
        logging.info("Configuration:")
        logging.info(textwrap.indent(self.profile_data, "    "))

        # Prepare runtime environment
        logging.info(f"Runtime directory: {self.runtime_dir}")

    def _read_conf_profile(self, conf: Configuration) -> None:
        self.profile = conf.current_profile
        self.profile_path = conf.profile_path(self.profile)
        self.profile_data = conf.read(self.profile)

    def _read_anonymous_profile(self, conanfile: str) -> None:
        logging.info(f"Source profile: {conanfile}")
        self.profile_path = conanfile
        with open(conanfile, "r", encoding="utf-8") as file:
            self.profile_data = file.read()
        hasher = hashlib.blake2b(digest_size=20)
        hasher.update(self.profile_data.encode())
        self.profile = hasher.hexdigest()

    def runtime_env_path(self) -> Path:
        """Return the path to the list of pruned environment variables."""
        return self.runtime_dir / "environment_all.sh.env"

    def _prepare_runtime_dir(self, with_json: bool = False) -> None:
        """Clean and create runtime directory."""
        self.clean()
        logging.debug(f"Create: {self.runtime_dir}")
        self.runtime_dir.mkdir(parents=True)
        self._prepare_virtualenv(with_json)
        self._write_cloe_env()
        self._write_activate_all(
            [
                # From Conan VirtualRunEnv (!= virtualrunenv) generator:
                self.runtime_dir
                / "conanrun.sh",
            ],
            [
                # From Conan virtualenv generator:
                self.runtime_dir / "environment.sh.env",
                # From Conan virtualrunenv generator:
                self.runtime_dir / "environment_run.sh.env",
                # From Conan base package cloe-launch-profile:
                self.runtime_dir / "environment_cloe_launch.sh.env",
                # From self._write_cloe_env(), derived from environment_run.sh:
                self.runtime_dir / "environment_cloe.sh.env",
            ],
        )
        self._write_prompt_sh()
        self._write_bashrc()
        self._write_zshrc()

    def _write_prompt_sh(self) -> None:
        """Write prompt.sh file."""
        prompt_sh_file = self.runtime_dir / "prompt.sh"
        prompt_sh_data = textwrap.dedent(
            """\
            if [[ -n $ZSH_VERSION ]]; then
                export PS1="%{%F{242}%}[cloe-shell]%{%f%} ${PS1}"
            else
                export PS1="\u001b[2m[cloe-shell]\u001b[0m $PS1"
            fi
            """
        )
        logging.debug(f"Write: {prompt_sh_file}")
        with prompt_sh_file.open("w") as file:
            file.write(prompt_sh_data)

    def _write_bashrc(self) -> None:
        """Write .bashrc file.

        When launching a shell, this file is used as the Bash user configuration.
        This is necessary to properly override the prompt with [cloe-shell] prefix.
        """
        bashrc_file = self.runtime_dir / ".bashrc"
        bashrc_data = textwrap.dedent(
            """\
            source /etc/bash.bashrc
            if [ -f ~/.bashrc ]; then
                source ~/.bashrc
            fi
            OLD_PS1="$PS1"
            source "$(dirname "$BASH_SOURCE[0]")/activate_all.sh"
            PS1="$OLD_PS1"
            source "$(dirname "$BASH_SOURCE[0]")/prompt.sh"
            """
        )
        logging.debug(f"Write: {bashrc_file}")
        with bashrc_file.open("w") as file:
            file.write(bashrc_data)

    def _write_zshrc(self) -> None:
        """Write .zshrc file.

        When launching a shell, this file is used as the Zsh user configuration.
        This is necessary to properly override the prompt with [cloe-shell] prefix.
        """
        zshrc_file = self.runtime_dir / ".zshrc"
        zshrc_data = textwrap.dedent(
            f"""\
            ZDOTDIR="${{OLD_ZDOTDIR:-$HOME}}"
            for file in "$ZDOTDIR/.zshenv" "$ZDOTDIR/.zprofile" "$ZDOTDIR/.zshrc"; do
                if [[ -f "$file" ]]; then
                    source "$file"
                fi
            done
            OLD_PS1="$PS1"
            source "{self.runtime_dir}/activate_all.sh"
            PS1="$OLD_PS1"
            source "{self.runtime_dir}/prompt.sh"
            """
        )
        logging.debug(f"Write: {zshrc_file}")
        with zshrc_file.open("w") as file:
            file.write(zshrc_data)

    def _write_cloe_env(self) -> None:
        """Derive important CLOE_ variables and write environment_cloe.sh.env file."""
        conanrun = (
            self.runtime_dir / "conanrun.sh"
        )  # From newer VirtualRunEnv generator
        activate_run = (
            self.runtime_dir / "activate_run.sh"
        )  # From older virtualrunenv generator
        if conanrun.exists():
            if activate_run.exists():
                logging.warning(
                    "Warning: Found both conanrun.sh and activate_run.sh in runtime directory!"
                )
                logging.warning("Note:")
                logging.warning(
                    "  It looks like /both/ VirtualRunEnv and virtualrunenv generators are being run."
                )
                logging.warning(
                    "  This may come from using an out-of-date cloe-launch-profile package."
                )
                logging.warning("")
                logging.warning(
                    "  Continuing with hybrid approach. Environment variables may be incorrectly set."
                )
            env = Environment(conanrun, source_file=True)
        elif activate_run.exists():
            env = Environment(activate_run, source_file=True)
        else:
            raise RuntimeError(
                "cannot find conanrun.sh or activate_run.sh in runtime directory"
            )

        if env.has("CLOE_SHELL"):
            logging.error("Error: recursive cloe shells are not supported.")
            logging.error("Note:")
            logging.error(
                "  It appears you are already in a Cloe shell, since CLOE_SHELL is set."
            )
            logging.error(
                "  Press [Ctrl-D] or run 'exit' to quit this session and then try again."
            )
            if self.abort_recursive_shell:
                sys.exit(2)

        cloe_env = Environment({})
        cloe_env.set("CLOE_SHELL", self.runtime_env_path())
        cloe_env.set("CLOE_PROFILE_HASH", self.profile)
        cloe_env.set("CLOE_ENGINE", self._extract_engine_path(env))
        cloe_env.path_set("CLOE_PLUGIN_PATH", self._extract_plugin_paths(env))
        cloe_env.export(self.runtime_dir / "environment_cloe.sh.env")

    def _write_activate_all(
        self, source_files: List[Path], env_files: List[Path]
    ) -> None:
        """Write activate_all.sh file."""
        activate_file = self.runtime_dir / "activate_all.sh"
        activate_data = textwrap.dedent(
            """\
            export_vars_from_file() {
                if [ ! -f "$1" ]; then
                    return
                fi
                while read -r line; do
                    LINE="$(eval echo $line)"
                    export "$LINE"
                done < "$1"
            }

            """
        )

        for file in source_files:
            if not file.exists():
                logging.info(f"Skipping source: {file}")
            filename = shlex.quote(file.as_posix())
            activate_data += f". {filename}\n"

        for file in env_files:
            if not file.exists():
                logging.info(f"Skipping export: {file}")
            filename = shlex.quote(file.as_posix())
            activate_data += f"export_vars_from_file {filename}\n"
        activate_data += "\nunset export_vars_from_file\n"

        logging.debug(f"Write: {activate_file}")
        with activate_file.open("w") as file:
            file.write(activate_data)

    def _prepare_virtualenv(self, with_json: bool = False) -> None:
        # Get conan to create a virtualenv AND virtualrunenv for us:
        # One gives us the LD_LIBRARY_PATH and the other gives us env_info
        # variables set in packages.
        conan_cmd = [
            str(self.conan_path),
            "install",
            "--install-folder",
            str(self.runtime_dir),
            "-g",
            "VirtualRunEnv",
        ]
        if with_json:
            conan_cmd.append("-g")
            conan_cmd.append("json")
        for arg in self.conan_args:
            conan_cmd.append(arg)
        conan_cmd.append(self.profile_path)
        result = subprocess.run(conan_cmd, check=False, capture_output=True)
        if result.returncode == 0:
            # Short-circuit out if everything is fine.
            return

        logging.error("Error: cannot install virtualenv configuration!")
        logging.error("Command:")
        logging.error(f"  {' '.join(conan_cmd)}")
        logging.error("")
        logging.error("Output:")

        stderr_lines = result.stderr.decode().splitlines()
        for line in stderr_lines:
            logging.error(
                "\n".join(textwrap.wrap(line, 80, initial_indent="  ", subsequent_indent="    ", break_long_words=False))
            )

        # Here are some errors that can happen and what to do about them.
        # NOTE: Tested with conan 1.60.1, may not match with other Conan versions.
        known_errors = {
            "^ERROR:.* Version range .* from requirement 'cloe-launch-profile/.* could not be resolved.*$": [
                "It looks like the cloe-launch-profile package hasn't been exported.",
                "",
                "Suggestion:",
                "  From the cloe repository, try performing the following commands:",
                "  ",
                "    cd cli",
                "    conan export .",
                "",
            ],
            "^ERROR: Failed requirement '([^']+)' from .*$": [
                "It looks like Conan does not know about one of the packages that",
                "the input configuration requires.",
                "",
                "This may be because:",
                "  - The specification of the requirement in the input configuration is wrong.",
                "  - The required package needs to be exported into Conan cache.",
                "  - The required package needs to be marked editable.",
                "In the last two cases, the package also needs to be built.",
                "",
                "Suggestion:",
                "  From the cloe repository, look at the output from `make help`.",
                "  There may be several targets that export the required packages.",
            ],
            "^ERROR: Missing prebuilt package for '([^']+)'.*$": [
                "It looks like Conan knows about all required packages, but they are not built.",
                "",
                "If you intend to use the required package in editable mode,",
                "you need to make this clear to Conan.",
                "",
                "Suggestion:",
                "  Use cloe-launch to build all necessary packages:",
                "  ",
                f"    cloe-launch prepare {self.profile_path}",
                "  ",
            ],
        }
        for error in stderr_lines:
            for (regex, response) in known_errors.items():
                if re.match(regex, error):
                    logging.error("")
                    logging.error("Note:")
                    for line in response:
                        logging.error(f"  {line}")
        sys.exit(2)


    def _extract_engine_path(self, env: Environment) -> Path:
        """Return the first cloe-engine we find in the PATH."""
        for bindir in env.get_list("PATH", default=[]):
            pp = Path(bindir) / "cloe-engine"
            if pp.exists():
                return pp

        logging.error("Error: cannot locate cloe-engine exectuable!")
        logging.error("Note:")
        logging.error("  This problem usually stems from one of two common errors:")
        logging.error("  - The conanfile for cloe-launch does not require cloe-engine.")
        logging.error(
            "  - The cloe-engine package or binary has not been built / is corrupted."
        )
        logging.error(
            "  However, unconvential or unsupported package configuration may also trigger this."
        )
        logging.error("")
        logging.error("Note: PATH contains these directories:")
        for bindir in env.get_list("PATH", default=[]):
            logging.error(f"  {bindir}")
        sys.exit(2)

    def _extract_plugin_paths(self, env: Environment) -> List[Path]:
        """Return all Cloe plugin paths we find in LD_LIBRARY_PATH."""
        plugin_paths = []
        for libdir in env.get_list("LD_LIBRARY_PATH", default=[]):
            pp = Path(libdir) / "cloe"
            if pp.exists():
                plugin_paths.append(pp)
        return plugin_paths

    def _extract_plugin_setups(self, lib_paths: List[Path]) -> List[Type[PluginSetup]]:
        for lib_dir in lib_paths:
            for file in lib_dir.iterdir():
                if not file.suffix == ".py":
                    continue
                path = lib_dir / file
                logging.info(f"Loading plugin setup: {path}")
                _find_plugin_setups(path)
        return PluginSetup.__subclasses__()

    def _prepare_runtime_env(
        self, use_cache: bool = False, with_json: bool = False
    ) -> Environment:
        if self.runtime_env_path().exists() and use_cache:
            logging.debug("Re-using existing runtime directory.")
        else:
            logging.debug("Initializing runtime directory ...")
            self._prepare_runtime_dir(with_json=with_json)

        # Get environment variables we need:
        return Environment(
            self.runtime_dir / "activate_all.sh",
            preserve=None if not self.preserve_env else list(os.environ.keys()),
            source_file=True,
        )

    def _write_runtime_env(self, env: Environment) -> None:
        logging.debug(f"Write: {self.runtime_env_path()}")
        env.export(self.runtime_env_path())

    def _process_arg(self, src_path) -> str:
        # FIXME: It may be possible to replace this with the Popen
        # pass_fds argument.

        if not re.match(self.anonymous_file_regex, src_path):
            return src_path

        dst_file = src_path.replace("/", "_")
        dst_path = self.runtime_dir / dst_file
        logging.info(f"Relay anonymous file {src_path} into {dst_path}")
        with open(src_path, "rb") as src:
            with open(dst_path, "wb") as dst:
                dst.write(src.read())
        return dst_path

    def _engine_cmd(self, args) -> List[str]:
        result = [self.engine_path]
        result.extend(self.engine_pre_args)

        # When invoking cloe-launch with a line like:
        #
        #  cloe-launch exec -- dump <(cat your-file) <(cat next-file)
        #
        # we need to read these files, which are in the format `/proc/self/fd/XYZ`,
        # store them in the runtime directory, and pass them on to the cloe
        # executable.
        if self.relay_anonymous_files:
            result.extend([self._process_arg(a) for a in args])
        else:
            result.extend(args)
        result.extend(self.engine_post_args)
        return result

    def _prepare_plugin_setups(self, env: Environment) -> List[PluginSetup]:
        # Augment environment:
        plugin_paths = self._extract_plugin_paths(env)
        plugin_setup_types = self._extract_plugin_setups(plugin_paths)
        plugin_setups = []
        for setup_type in plugin_setup_types:
            setup = setup_type(env)
            setup.environment()
            plugin_setups.append(setup)

        return plugin_setups

    def clean(self) -> None:
        """Clean the runtime directory."""
        if self.runtime_dir.exists():
            logging.debug(f"Remove: {self.runtime_dir}")
            shutil.rmtree(self.runtime_dir, ignore_errors=True)

    def shell(
        self,
        arguments: Optional[List[str]] = None,
        use_cache: bool = False,
    ) -> None:
        """Launch a SHELL with the environment variables adjusted."""
        env = self._prepare_runtime_env(use_cache)
        self._write_runtime_env(env)

        plugin_setups = self._prepare_plugin_setups(env)
        shell = os.getenv("SHELL", "/bin/bash")

        # Print the final environment, if desired
        logging.debug(f"Environment: {env}")

        if len(plugin_setups) > 0:
            logging.warning("Warning:")
            logging.warning(
                "  The following plugin drivers may contain setup() and teardown()"
            )
            logging.warning("  that will not be called automatically within the shell.")
            logging.warning("")
            for plugin in plugin_setups:
                logging.warning(f"    {plugin.plugin}")

        # Replace this process with the SHELL now.
        sys.stdout.flush()
        cmd = [shell]
        if shell == "/bin/bash":
            cmd.extend(["--init-file", str(self.runtime_dir / ".bashrc")])
        elif shell == "/bin/zsh":
            env.set("OLD_ZDOTDIR", str(env.get("ZDOTDIR", "")))
            env.set("ZDOTDIR", self.runtime_dir)
        else:
            # NOTE: This will also happen if "bash" or "zsh" happen to be
            # at a different location than expected here. This is not a
            # logging.warn, because it's a minor problem, and we don't
            # want to pollute the output with this statement every time.
            logging.info(f"Warning: unsupported shell '{shell}'")
        if arguments is not None:
            cmd.extend(arguments)
        logging.debug(f"Exec: {' '.join(cmd)}")
        os.execvpe(shell, cmd, env.as_dict())

    def activate(
        self,
        use_cache: bool = False,
    ) -> None:
        """Print shell commands to activate a cloe-engine environment."""
        env = self._prepare_runtime_env(use_cache)
        self._write_runtime_env(env)

        print("# Please see `cloe-launch activate --help` before activating this.")
        print()
        print(f"source {self.runtime_dir / 'activate_all.sh'}")
        print(f"source {self.runtime_dir / 'prompt.sh'}")

    def _set_build_policy(self, build_policy: Optional[List[str]]):
        if build_policy is None:
            build_policy = ["outdated", "cascade"]
        for policy in build_policy:
            if policy == "":
                self.conan_args.append("--build")
            else:
                self.conan_args.append(f"--build={policy}")

    def prepare(self, build_policy: Optional[List[str]] = None) -> None:
        """Prepare (by downloading or building) dependencies for the profile."""
        self.capture_output = False
        self._set_build_policy(build_policy)
        self._prepare_runtime_env(use_cache=False)

    def deploy(
        self,
        dest: Path,
        wrapper: Optional[Path] = None,
        wrapper_target: Optional[Path] = None,
        patch_rpath: bool = True,
        build_policy: Optional[List[str]] = None,
    ) -> None:
        """Deploy dependencies for the profile."""
        self.capture_output = False
        self._set_build_policy(build_policy)
        self._prepare_runtime_env(use_cache=False, with_json=True)

        # Ensure destination exists:
        if not dest.is_dir():
            if dest.exists():
                logging.error(f"Error: destination is not a directory: {dest}")
                sys.exit(1)
            dest.mkdir(parents=True)

        # Copy necessary files to destination:
        # TODO: Create a manifest and be verbose about files being copied.
        build_info = self.runtime_dir / "conanbuildinfo.json"
        logging.info(f"Reading: {build_info}")
        build_data = json.load(build_info.open())
        install_manifest: List[Path] = []

        def copy_file(src, dest):
            install_manifest.append(Path(dest))
            logging.info(f"Installing: {dest}")
            return shutil.copy2(src, dest)

        def copy_tree(src, dest, ignore):
            if src.find("/build/") != -1:
                logging.warning(
                    f"Warning: deploying from build directory is strongly discouraged: {dep['rootpath']}"
                )
            shutil.copytree(
                src,
                dest,
                copy_function=copy_file,
                dirs_exist_ok=True,
                ignore=shutil.ignore_patterns(*ignore),
            )

        for dep in build_data["dependencies"]:
            for src in dep["bin_paths"]:
                copy_tree(src, dest / "bin", ignore=["bzip2"])
            for src in dep["lib_paths"]:
                copy_tree(src, dest / "lib", ignore=["cmake", "*.a"])

        # Patching RPATH of all the binaries lets everything run
        # fine without any extra steps, like setting LD_LIBRARY_PATH.
        if patch_rpath:
            assert platform.system() != "Windows"
            if shutil.which("patchelf") is None:
                logging.error("Error: required executable is not available: patchelf")
                sys.exit(2)
            binutils.patch_binary_files_rpath(dest / "bin", ["$ORIGIN/../lib"])
            binutils.patch_binary_files_rpath(dest / "lib" / "cloe", ["$ORIGIN/.."])

        if wrapper is not None:
            if wrapper_target is None:
                wrapper_target = dest / "bin" / "cloe-engine"
            wrapper_data = textwrap.dedent(
                f"""\
                #!/bin/sh

                {wrapper_target} $@
                """
            )
            with wrapper.open("w") as wrapper_file:
                wrapper_file.write(wrapper_data)

        def simplify_manifest(manifest: Set[Path]):
            for path in list(manifest):
                parent = path.parent
                while parent != parent.parent:
                    if parent in manifest:
                        manifest.remove(parent)
                    parent = parent.parent

        # Create uninstaller from manifest
        uninstaller_file = self.runtime_dir / "uninstall.sh"
        logging.info(f"Write: {uninstaller_file}")
        with uninstaller_file.open("w") as f:
            install_dirs: Set[Path] = set()
            f.write("#!/bin/bash\n")
            for file in install_manifest:
                install_dirs.add(file.parent)
                f.write(f"echo 'Removing file: {file}'\n")
                f.write(f"rm '{file}'\n")
            simplify_manifest(install_dirs)
            for path in install_dirs:
                f.write(f"echo 'Removing dir: {path}'\n")
                f.write(f"rmdir -p '{path}'\n")

    def exec(
        self,
        args: List[str],
        use_cache: bool = False,
        debug: bool = False,
        override_env: Optional[Dict[str, str]] = None,
    ) -> subprocess.CompletedProcess:
        """Launch cloe-engine with the environment variables adjusted and with
        plugin setup and teardown."""
        env = self._prepare_runtime_env(use_cache)
        self._write_runtime_env(env)
        plugin_setups = self._prepare_plugin_setups(env)

        # Initialize plugin setups:
        for plugin in plugin_setups:
            logging.debug(
                f"Initializing plugin setup for {plugin.name} at {plugin.plugin}"
            )
            plugin.setup()

        # Override environment setup.
        if override_env is not None:
            for k in override_env.keys():
                env[k] = override_env[k]

        # Print the final environment, if desired
        logging.debug(f"Environment: {env}")

        # Run cloe engine:
        self.engine_path = env["CLOE_ENGINE"]
        cmd = self._engine_cmd(args)
        if debug:
            cmd.insert(0, "gdb")
            cmd.insert(1, "--args")
        logging.info(f"Exec: {' '.join(cmd)}")
        logging.info("---")
        print(end="", flush=True)
        result = subprocess.run(cmd, check=False, env=env.as_dict())
        for setup in plugin_setups:
            setup.teardown()
        return result

    def _run_cmd(self, cmd, must_succeed=True) -> subprocess.CompletedProcess:
        return procutils.system(
            cmd, must_succeed=must_succeed, capture_output=self.capture_output
        )
