from typing import List, Optional, Dict
import subprocess


def run_cmd(
    cmd: List[str],
    env: Optional[Dict[str, str]] = None,
    must_succeed: bool = True,
    verbose: bool = False,
) -> subprocess.CompletedProcess:
    """Run a command quietly, only printing stderr if the command fails."""

    if verbose:
        print("Exec:", " ".join(cmd))
    result = subprocess.run(
        cmd,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
        env=env,
    )
    if result.returncode != 0:
        print("Error running:", " ".join(cmd))
        print(result.stdout)
        if must_succeed:
            raise ChildProcessError()
    return result
