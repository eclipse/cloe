import logging
import subprocess
import platform

from pathlib import Path
from typing import List, Optional

from cloe_launch import procutils


def patch_rpath(file: Path, rpath: List[str]) -> Optional[subprocess.CompletedProcess]:
    """Set the RPATH of an executable or library.

    This will fail silently if the file is not an ELF executable.
    """
    assert platform.system() != "Windows"
    assert file.exists()

    # Get original RPATH of file, or fail.
    cmd = ["patchelf", "--print-rpath", str(file)]
    result = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False
    )
    if result.returncode != 0:
        if result.stderr.decode().strip() == "not an ELF executable":
            # Silently ignore binaries that don't apply.
            return None
        logging.error(f"Error running: {' '.join(cmd)}")
        logging.error(result.stderr)
    original = result.stdout.decode().strip()
    if original != "":
        rpath.append(original)

    file_arg = str(file)
    rpath_arg = ":".join(rpath)
    logging.debug(f"Setting RPATH: {file_arg} -> {rpath_arg}")
    return procutils.system(["patchelf", "--set-rpath", rpath_arg, file_arg], must_succeed=False)


def find_binary_files(cwd: Optional[Path] = None) -> List[Path]:
    """Return a list of all file paths that are of the binary type."""
    assert platform.system() != "Windows"
    result = subprocess.run(
        """find -type f -exec sh -c "file -i '{}' | grep -q '; charset=binary'" \\; -print""",
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
        logging.info(f"Patching RPATH: {file}")
        patch_rpath(file, rpath)
