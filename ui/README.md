# Cloe UI

Cloe UI is a web-based graphical user interface for Cloe. It requires that the
internal web server in Cloe is enabled, as it gets its data from the JSON web
API.

## Getting Started

The fastest way to get started is to build a Docker image and run that:

    `make image run`

The build may get stuck due to the default Ubuntu open file limit of 1024.
Increase ulimit as described here: https://superuser.com/a/1200818

Also, it might be needed to restart the cntlm service.

Note that a Cloe UI version has a minimum of two docker tags: The version tag
(`cloe-ui:version-git_commit_hash`) and the cloe-compatibility tag
(`cloe-ui:cloe_version`).

## Usage

### Connecting to Cloe

After starting the docker container, Cloe UI is accessible on `http://localhost:5000`.

By default, Cloe-WebUI tries to connect to a Cloe Host on `localhost:8080`. You
can change that in the Navigation Bar of the UI. Alternatively, you can specify
a Cloe Host via GET Parameter:
`http://localhost:5000/?host=cloe-host.example.com:8080`
