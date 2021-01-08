"""
Run a specific Cloe configuration profile.

This script helps run Cloe instances by setting up a Conan virtualrunenv in
the cache and running cloe-engine from there. This is one way the Cloe CLI
might work in the future. Once the Cloe CLI is in-use, we deprecate and
remove this script.

Commands:
  exec      Run cloe-engine with the given arguments.
  show      Show default/specified profile.
  list      List the currently available profiles.
  add       Add a profile with the given name.
  edit      Edit the default/specified profile with $EDITOR.
  remove    Remove the specified profile.
  default   Get or set the default profile.

"""

from typing import List, Dict
import os
import sys
import click
import logging

from cloe_launch.exec import Engine
from cloe_launch import Configuration, ConfigurationError


@click.group()
@click.option(
    "-v",
    "--verbose",
    envvar="CLOE_VERBOSE",
    count=True,
    help="Print more information (mostly debugging).",
)
@click.pass_context
def main(ctx, verbose: int):
    """Launch cloe-engine with profiles and manage launch profiles."""

    if verbose == 0:
        level = logging.WARNING
    elif verbose == 1:
        level = logging.INFO
    else:
        level = logging.DEBUG
    logging.basicConfig(format="%(message)s", stream=sys.stderr, level=level)

    class Options:
        def __init__(self, verbose):
            self.verbose = verbose

    ctx.obj = Options(verbose > 0)


# Common options among engine and profile commands:
def profile_option(
    required: bool = False, help="Profile to select, default if absent."
):
    conf = Configuration()

    def complete(ctx, args, incomplete):
        profiles = []
        for k in conf.all_profiles:
            if k not in args:
                profiles.append(k)
        return [k for k in profiles if incomplete in k]

    return click.option(
        "-p",
        "--profile",
        envvar="CLOE_PROFILE",
        required=required,
        type=click.STRING,
        help=help,
        autocompletion=complete,
    )


def profile_path_option():
    return click.option(
        "-P",
        "--profile-path",
        envvar="CLOE_PROFILE_PATH",
        type=click.Path(exists=True, file_okay=True, dir_okay=False),
        help="Conanfile to use as anonymous profile.",
    )


def deny_profile_and_path(profile: str, profile_path: str) -> None:
    """Assert that --profile and --profile-path are not specified simultaneously."""
    if profile is not None and profile_path is not None:
        raise click.UsageError(
            "--profile and --profile-path options cannot be specified simultaneously"
        )


# _________________________________________________________________________
# Command: exec [--cache] [--debug] [--profile=PROFILE | --profile-path=CONANFILE]
#               [--] ENGINE_ARGS
@main.command("exec")
@click.argument("engine_args", nargs=-1)
@profile_option()
@profile_path_option()
@click.option(
    "-c", "--cache/--no-cache", is_flag=True, help="Re-use the cache if available."
)
@click.option("-d", "--debug", is_flag=True, help="Launch cloe-engine with GDB.")
@click.option("-E", "--preserve-env", is_flag=True, help="Preserve user environment.")
@click.option(
    "-o",
    "--conan-option",
    type=click.STRING,
    multiple=True,
    help="Options to pass to Conan for virtualrunenv generation",
)
@click.option(
    "-e",
    "--override-env",
    multiple=True,
    type=click.STRING,
    help="Use environment variable as set or preserve.",
)
@click.pass_obj
def cli_exec(
    opt,
    engine_args: List[str],
    profile: str,
    profile_path: str,
    preserve_env: bool,
    override_env: List[str],
    conan_option: List[str],
    cache: bool,
    debug: bool,
) -> None:
    """Launch cloe-engine with a profile.

    ENGINE_ARGS are passed on to cloe-engine.
    """
    deny_profile_and_path(profile, profile_path)
    conf = Configuration(profile)
    engine = Engine(conf, conanfile=profile_path)
    engine.preserve_env = preserve_env
    engine.conan_options = conan_option

    # Prepare environment overrides:
    overrides = {}
    for line in override_env:
        kv = line.split("=", 1)
        if len(kv) == 1:
            kv.append(os.getenv(kv[0], ""))
        overrides[kv[0]] = kv[1]

    # Run cloe-engine and pass on returncode:
    # If cloe-engine is killed/aborted, subprocess will return 250.
    result = engine.exec(
        engine_args, use_cache=cache, debug=debug, override_env=overrides
    )
    sys.exit(result.returncode)


# _________________________________________________________________________
# Command: shell [--cache] [--profile=PROFILE | --profile-path=CONANFILE]
@main.command("shell")
@profile_option()
@profile_path_option()
@click.option(
    "-c", "--cache/--no-cache", is_flag=True, help="Re-use the cache if available."
)
@click.option("-E", "--preserve-env", is_flag=True, help="Preserve user environment.")
@click.option(
    "-o",
    "--conan-option",
    type=click.STRING,
    multiple=True,
    help="Options to pass to Conan for virtualrunenv generation",
)
@click.argument("shell_args", nargs=-1)
@click.pass_obj
def cli_shell(
    opt,
    shell_args: List[str],
    profile: str,
    profile_path: str,
    preserve_env: bool,
    conan_option: List[str],
    cache: bool,
) -> None:
    """Launch shell with the correct environment from a profile."""
    deny_profile_and_path(profile, profile_path)
    conf = Configuration(profile)
    engine = Engine(conf, conanfile=profile_path)
    engine.preserve_env = preserve_env
    engine.conan_options = conan_option

    # Replace process with shell.
    engine.shell(shell_args, use_cache=cache)


# _________________________________________________________________________
# Command: clean [--profile PROFILE | --profile-path=CONANFILE]
@main.command("clean")
@profile_option()
@profile_path_option()
@click.pass_obj
def cli_clean(opt, profile: str, profile_path: str) -> None:
    """Clean launcher profile cache."""
    deny_profile_and_path(profile, profile_path)
    conf = Configuration(profile)
    engine = Engine(conf, conanfile=profile_path)
    engine.clean()


# _________________________________________________________________________
# Command: profile (show | list | add | edit | remove | default)
@main.group("profile")
def cli_profile():
    """Manage launcher profiles."""


# _________________________________________________________________________
# Command: profile show [--profile PROFILE]
@cli_profile.command("show")
@profile_option()
@click.pass_obj
def cli_profile_show(opt, profile: str) -> None:
    """Show a profile configuration."""
    conf = Configuration(profile)
    if conf.current_profile is None:
        raise ConfigurationError("no default profile is configured")
    data = conf.read(conf.current_profile)
    print(data)


# _________________________________________________________________________
# Command: profile list
@cli_profile.command("list")
@click.pass_obj
def cli_profile_list(opt) -> None:
    """List all available profiles."""
    conf = Configuration()
    for profile in conf.all_profiles:
        if opt.verbose:
            if profile == conf.default_profile:
                print("*", profile)
            else:
                print(" ", profile)
        else:
            print(profile)


# _________________________________________________________________________
# Command: profile add [--default] [--force] --profile PROFILE CONANFILE
@cli_profile.command("add")
@profile_option(required=True, help="Name of the profile to be added.")
@click.argument(
    "conanfile", type=click.Path(exists=True, file_okay=True, dir_okay=False)
)
@click.option("-d", "--default", is_flag=True, help="Set the new profile as default.")
@click.option("-f", "--force", is_flag=True, help="Overwrite an existing profile.")
@click.pass_obj
def cli_profile_add(
    opt, profile: str, conanfile: str, force: bool = False, default: bool = False
) -> None:
    """Add a new profile."""
    conf = Configuration()
    conf.add(profile, conanfile, force=force)
    if default:
        conf.set_default(profile)


# _________________________________________________________________________
# Command: profile edit [--editor EDITOR] [--no-create]
@cli_profile.command("edit")
@profile_option()
@click.option(
    "-e",
    "--editor",
    envvar="EDITOR",
    default=os.getenv("EDITOR"),
    show_default=True,
    help="Editor to use for editing the profile.",
)
@click.option(
    "--create/--no-create",
    is_flag=True,
    help="Create the profile if it does not exist.",
)
@click.pass_obj
def cli_profile_edit(opt, profile: str, editor: str, create: bool) -> None:
    """Edit a profile."""
    conf = Configuration(profile)
    if conf.current_profile is None:
        raise ConfigurationError("no default profile is configured")
    conf.edit(conf.current_profile, create)


# _________________________________________________________________________
# Command: profile remove --profile PROFILE
@cli_profile.command("remove")
@profile_option(required=True, help="Profile to remove.")
@click.pass_obj
def cli_profile_remove(opt, profile: str) -> None:
    """Remove a profile."""
    conf = Configuration()
    conf.remove(profile)


# _________________________________________________________________________
# Command: profile default [--profile PROFILE]
@cli_profile.command("default")
@profile_option()
@click.pass_obj
def cli_profile_default(opt, profile: str) -> None:
    """Show or set the default profile."""
    conf = Configuration(profile)
    if profile is None:
        if conf.default_profile is not None:
            print(conf.default_profile)
        elif opt.verbose:
            print("Note: no default profile is configured")
    else:
        # Set default to provided value, which must exist already
        conf.set_default(profile)


if __name__ == "__main__":
    main()
