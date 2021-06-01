Eclipse Cloe
============

Cloe empowers developers of automated-driving software components by providing
a unified interface to closed-loop simulation. It achieves this by abstracting
over environment and vehicle simulators and building upon these.

![Screenshot of the Cloe UI](docs/images/screenshot.png)

Cloe isn't a single tool. It is a set of components written in different
languages employing different tools that work together:

 - Cloe runtime & engine (C++)
 - Cloe command line interface (Python)
 - Cloe web user interface (Javascript/React)

Contributing to the CLI and the UI is pretty straightforward, as these are
tools that live more-or-less 100% in their respective subdirectories. The
runtime and engine are a different story though.

Cloe is meant to be extended through plugins that build on the Cloe runtime and
are integrated at runtime by the Cloe engine. These are written in C++, which
doesn't have a defacto method of packaging. Finding a solution for integrating
C++ packages is always going to be a trade-off between ease-of-use for
developers and users. In our experience, it is better to require more from
developers than from users, as demanding too much from users usually ends up
backfiring with increased support issues.

Getting Started
---------------

For building, deploying, and running the runtime and engine we use [Conan][1],
a modern C++ package manager. We currently have not published any Conan
packages that can be downloaded directly. Building them yourself is pretty
straightforward.

Currently, we only support Linux or [WSL][2].

### Installing Dependencies

We provide automatic dependency installation for [Ubuntu][3] und [Archlinux][4]
via the `Makefile.setup` Makefile. You should inspect it before
running the targets, as these will modify your system.
Other distributions may work, if the packages are available.

    git clone https://github.com/eclipse/cloe.git
    cd cloe
    sudo make install-system-deps
    make install-python-deps

You may need to setup your Conan profile before continuing on to the next
point. In a pinch, the following steps should suffice:

 1. Install Conan with Python.
    ```
    pip3 install --user --upgrade conan
    ```
 2. Define a Conan profile, which defines the machine configuration.
    ```
    conan profile new --detect default
    conan profile update settings.compiler.libcxx=libstdc++11 default
    ```
    If everything works out, your Conan profile should look something like this.
    ```console
    $ conan profile show default
    Configuration for profile default:
    [settings]
      os               = Linux
      os_build         = Linux
      arch             = x86_64
      arch_build       = x86_64
      compiler         = gcc
      compiler.version = 9
      compiler.libcxx  = libstdc++11
      build_type       = Release
    ```

3. Increase the request timeout to work around performance [issues][5] with the
   Conan Center.
   ```
   conan config set general.request_timeout=360
   ```
See the Conan [documentation][6] for more information on how to do this.

### Building the Cloe Packages

To build all packages, you should run the following:

    make export-vendor
    make package-auto

This will export all Conan recipes from this repository and create the cloe
package. Conan will download and build all necessary dependencies. Should
any errors occur during the build, you may have to force Conan to build
all packages instead of re-using packages it finds:
```
    make package-all
```
Run `make help` to get an overview of the available targets we expect you to
use. For more details on how this is done, have a look at the Makefiles in the
repository root or the Dockerfiles in `dist/docker` directory.

If you experience timeout issues waiting for Conan Center, the reason is likely
the boost dependency's hundreds of binary packages. You can then slightly
increase Conan's timeout configuration like so:

    export CONAN_REQUEST_TIMEOUT=320

### Running Cloe

Since Cloe is made up of many packages, running the Cloe engine directly is
somewhat tricky. Conan provides the `virtualrunenv` generator, which creates
shell scripts that you can source, similar to Python's virtualenv. Or, you can
use the `cloe-launch` tool, in the `cli` directory, which wraps all this
functionality for you in one convenient place.

You can install an editable instance with `pipx` (or `pip`):

    cd cli
    pipx install -e .

This has the advantage that any updates to the repository will be transparently
used.

Once the `cloe-launch` tool is available, you can do one of the following:

 1. Launch a shell with the environment adjusted:
    ```console
    $ cloe-launch -v shell -P conanfile.py
    Source profile: conanfile.py
    Profile name: 5990582c3331e43d46d7e40f293a53b76063f1e4
    Configuration:
        ...
    Runtime directory: /home/captain/.cache/cloe/launcher/5990582c3331e43d46d7e40f293a53b76063f1e4
    $ cloe-engine usage
    Cloe 0.18.0-nightly (2020-10-01)
    ...
    ```
 2. Launch `cloe-engine` directly:
    ```console
    $ cloe-launch -v exec -P conanfile.py -- usage
    Source profile: conanfile.py
    Profile name: 5990582c3331e43d46d7e40f293a53b76063f1e4
    Configuration:
        ...
    Runtime directory: /home/captain/.cache/cloe/launcher/5990582c3331e43d46d7e40f293a53b76063f1e4
    ---
    Cloe 0.18.0-nightly (2020-10-01)
    ...
    ```

[1]: https://conan.io
[2]: https://docs.microsoft.com/en-us/windows/wsl/about
[3]: https://ubuntu.com
[4]: https://archlinux.org
[5]: https://github.com/conan-io/conan-center-index/issues/950
[6]: https://docs.conan.io/en/latest/
