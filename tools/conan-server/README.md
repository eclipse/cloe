Dockerized Conan Server
=======================

This directory contains a Dockerized Conan server which can be used for testing
and development of the Cloe Conan packages.

It is possible to take this and use it as a Conan repository for production
purposes, but it is explicitly discouraged by the Conan documentation.
Instead, you should use Artifactory for this purpose.

Usage
-----

Check the `.env` and adjust the configuration if you want to. This file is used
by both the Makefile and Docker Compose. Build the Docker container:

    docker-compose build

When you are finished with that, you can start the image:

    docker-compose up -d

Now you have a Conan server running on http://localhost:9300. You can add this as
a remote to your Conan configuration:

    conan remote add local http://localhost:9300

This will let you read and download packages from the remote, but you won't be
able to upload any packages until you authenticate with the server as a user.
The default `cloe-stable` user is allowed to push all sorts of packages, which
is useful for populating the remote with dependencies for Cloe. Use the password
that is set in the `server.conf` file.

    conan user -r local cloe-stable -p

Now you can upload all the packages you want with:

    conan upload -r local --all --confirm boost/\*
    conan upload -r local --all --confirm spdlog/\*
    ...

Because this can be an arduous task if you want to upload many packages,
I recommend something along the lines of the following procedure:

    conan search --raw > packages.txt
    $EDITOR packages.txt
    cat packages.txt | while read package; do \
        conan upload -r local --all --confirm $package; \
    done

Have a look at the scripts in this folder for more inspiration.
