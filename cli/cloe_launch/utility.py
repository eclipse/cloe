import logging
import subprocess
import platform

from pathlib import Path
from typing import Dict, List, Optional


def run_cmd(
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


def patch_rpath(file: Path, rpath: List[str]):
    """Set the RPATH of an executable or library.

    This will fail silently if the file is not an ELF executable.
    """
    assert platform.system() != "Windows"
    assert file.exists()

    # Get original RPATH of file, or fail.
    result = subprocess.run(
        ["patchelf", "--print-rpath", str(file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.stderr.decode().strip() == "not an ELF executable":
        # Silently ignore binaries that don't apply.
        return
    original = result.stdout.decode().strip()
    if original != "":
        rpath.append(original)

    file_arg = str(file)
    rpath_arg = ":".join(rpath)
    logging.debug(f"Setting RPATH: {file_arg} -> {rpath_arg}")
    subprocess.run(
        ["patchelf", "--set-rpath", rpath_arg, file_arg],
        check=True,
    )


def find_binary_files(cwd: Optional[Path] = None) -> List[Path]:
    """Return a list of all file paths that are of the binary type."""
    assert platform.system() != "Windows"
    result = subprocess.run(
        """find -type f -exec sh -c "file -i '{}' | grep -q '; charset=binary'" \; -print""",
        shell=True,
        stdout=subprocess.PIPE,
        check=True,
        cwd=cwd,
    )
    return [Path(x) for x in result.stdout.decode().splitlines()]

def patch_binary_files_rpath(src: Path, rpath: List[str]):
    """Patch RPATH all binary files found in src directory."""
    for file in find_binary_files(src):
        file = src / file
        try:
            logging.info(f"Patching RPATH: {file}")
            patch_rpath(file, rpath)
        except:
            logging.warning(f"Error: cannot set RPATH of {file}")
