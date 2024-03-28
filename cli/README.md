# Cloe-Launch

The `cloe-launch` script is a stop-gap till the Cloe CLI (run as just `cloe`)
makes it into the repository.

## Installation

We recommend the use of [pipx][1] for installation, but you can also use
pip. The easiest way is to just use the `Makefile`:
```
make install
```
If you want to install in editable mode, you unfortunately need to disable
use of PEP-517 temporarily, which you can do by renaming `pyproject.toml`:
```
mv pyproject.toml{,.bak} && pipx install -e . && mv pyproject.toml{.bak,}
```
This is easiest with the `Makefile`, which automates the whole process:
```
make editable
```

## Usage

<!-- TODO: Update this usage! -->
```
cloe-launch [-v] clean                             CONANFILE

cloe-launch [-v] prepare                           CONANFILE [CONAN_INSTALL_ARGS]

cloe-launch [-v] activate [-c]                     CONANFILE [CONAN_INSTALL_ARGS]
    cloe-launch -v activate -c tests/conanfile.py -o cloe-engine:server=True

cloe-launch [-v] deploy  [-c] [-f] [-r] [-D PATH]  CONANFILE [CONAN_INSTALL_ARGS]

cloe-launch [-v] exec    [-c] [-e VAR] [-E] [-d]   CONANFILE [CONAN_INSTALL_ARGS] -- [ENGINE_ARGS]

    cloe-launch exec tests/conanfile.py -o cloe-engine:server=False -- -l debug run

cloe-launch [-v] shell   [-c] [-e VAR] [-E]        CONANFILE [CONAN_INSTALL_ARGS] -- [SHELL_ARGS]

```

## Design Considerations

The CLI interface to `cloe-engine` should do as little as possible and as much
as necessary. The order of operations should be clear.

  1. Convert YAML to JSON
  2. Relay STDIN and anonymous files
  3. Generate runtime environment from profile
  4. Run cloe-engine

### Multiple Sources

There are two main sources for combining plugins and data: local and
containerized. It should be possible to transparently combine these. This will
require replacing the profile format. It will probably also require extending
and refining the module.py plugin configuration method.

### TODO List

This is what still needs to be done:

 - [ ] Merge with Cloe CLI
 - [ ] Replace profile with more generic solution
 - [ ] Finalize module.py design
 - [ ] Allow combination of multiple sources

## Continuous Integration

One of the main use-cases of cloe-launch is to run a federation of Cloe Conan
packages. As such, we'd like to make use of the local instance of this Python
module without resorting to installing it first. This is possible by setting
the `PYTHONPATH` variable and using the `cloe_launch` module:

    PYTHONPATH="${CLOE_ROOT}/cli" python -m cloe_launch

[1]: https://pipxproject.github.io/pipx/
