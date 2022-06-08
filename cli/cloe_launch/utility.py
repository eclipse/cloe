import logging
import subprocess

from typing import Dict
from typing import List
from typing import Optional


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
        logging.error(result.stdout)
        if must_succeed:
            raise ChildProcessError()
    return result
