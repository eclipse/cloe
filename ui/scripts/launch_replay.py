#!/usr/bin/env python3

import webbrowser
import docker
import click
from pathlib import Path
import subprocess
from subprocess import CalledProcessError
from time import sleep


@click.command()
@click.option(
    "-p",
    "--path",
    type=click.Path(exists=True, file_okay=True, dir_okay=False),
    required=True,
    help="""Path to the replay file. The file needs to either be a .JSON or a
              .JSON.gz file. The file is generated if the api_recording flag is
              set in the Cloe stackfile""",
)
@click.option(
    "-h",
    "--host",
    default="localhost",
    help="IP adress of host where ui and server run. (Default is localhost)",
)
@click.option("--cloe-ui-image", required=True, help="Cloe-ui docker image.")
@click.option(
    "--webserver-image", required=True, help="Cloe-ui webserver docker image."
)
def main(path: str, host: str, cloe_ui_image: str, webserver_image: str):
    """Replay simulation data with the Cloe web-ui.
    Docker containers which are used:
        - cloe-ui
        - cloe-ui-webserver
    If there are no containers running, this script will start both.
    If the images are not found locally, they will be pulled from the repository.
    """

    data_file_path = Path(path).resolve()
    dirname = data_file_path.parent.name
    return_code = launch_cloe_ui_webserver(data_file_path, dirname, webserver_image)
    if return_code == 0:
        print(
            "-------------  Launched Webserver at http://localhost:4000 ------------- "
        )
    else:
        print(f"Failed to launch webserver: {return_code}")
        quit()

    return_code = launch_cloe_ui(cloe_ui_image)
    if return_code == 0:
        print("------------- Launched Cloe UI at http://localhost:5000 ------------- ")
    else:
        print(f"Failed to launch web ui: {return_code}")
        quit()

    sleep(3)
    open_browser(host, dirname, data_file_path.name)


def open_browser(host, dir, filename):
    url = f"http://{host}:5000?id={dir}&name={filename}&host={host}"
    webbrowser.open_new(url)


def launch_cloe_ui(docker_image):
    docker_container_name = "cloe-ui"
    # Stop cloe ui container if it is running.
    stop_container(docker_container_name)

    commands = []
    commands.extend(
        [
            "docker",
            "run",
            "--rm",
            "-d",
            f"--name={docker_container_name}",
            "-p",
            "5000:5000",
            docker_image,
        ]
    )

    return run_process(commands)


def stop_container(docker_container_name):
    docker_env = docker.from_env()
    running_container = docker_env.containers.list(
        filters={"status": "running", "name": docker_container_name}
    )
    for container in running_container:
        if container.name == docker_container_name:
            container.stop()


def launch_cloe_ui_webserver(mounting_path, dir, docker_image):
    docker_container_name = "cloe-ui-webserver"

    # Stop webserver container if it is running.
    stop_container(docker_container_name)

    commands = []
    commands.extend(
        [
            "docker",
            "run",
            "--rm",
            "-d",
            f"--name={docker_container_name}",
            "-p",
            "4000:4000",
            "-v",
            f"{str(mounting_path.parent)}:/app/replay_data/{dir}",
            docker_image,
        ]
    )

    return run_process(commands)


def run_process(commands: list, **kwargs) -> int:
    """Run a process with the given commands

    Return the return code in case of a thrown CalledProcessError, but doesn't raise.
    Raises Exception in case another exception was thrown.
    """
    try:
        subprocess.check_call(commands, cwd=kwargs.get("cwd"))
        return 0
    except CalledProcessError as exc:
        return exc.returncode
    except Exception as exc:
        print(f"Unexpected error occurred while running process: {exc}")
        raise


if __name__ == "__main__":
    main()
