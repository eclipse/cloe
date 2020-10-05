# Cloe-Launch

The `cloe-launch` script is a stop-gap till the Cloe CLI (run as just `cloe`)
makes it into the repository.

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

    PYTHONPATH="${CLOE_ROOT}/cli" cloe_launch
