# Version 0.25.0 Release

This release contains the initial implementation of Lua support with the new
DataBroker and Signals concept.

## Add Lua support

Lua is a lightweight embedded language that can now be used to define simulation
configuration and test execution and evaluation.

For example:

```lua
cloe = require("cloe")
proj = require("project")
cloe.load_stackfile("existing_configuration.json")
cloe.schedule_test {
    id = "e03fc31f-586b-4e57-80fa-ff2cba5ff9dd",
    on = events.start(),
    run = function(test, sync)
        cloe.log("info", "Entering test")
        test:wait_duration("1s")

        -- Enable Cruise Control
        proj.enable_hmi()
        test:wait_duration("4s")

        -- Set speed and increase
        local speed = cloe.signal(proj.Signals.VehicleSpeed).kmph()
        proj.buttons.plus.push("1500ms")
        test:wait_duration("4s")

        -- Check that new speed is higher than before but lower than
        -- the speed-limit.
        local new_speed = cloe.signal(proj.Signals.VehicleSpeed).kmph()
        test:assert_gt(new_speed, speed)
        test:assert_le(new_speed, proj.scenario.speed_limit)
        test:succeed()
    end
}
```

The above test-case is a hypothetical, yet realistic way that such a test can
be written. The results are written to an output file and the terminal output
in JSON format, so they can be easily processed.

The engine comes with a new `shell` command, which allows you to launch an
interactive Lua session or just run Lua files.

A simulation with or without Lua can be run as before with the `run` command.
The JSON stackfile interface is still supported and can be mixed with Lua.
In particular, all the features in the stackfiles can be now used from Lua
in a backwards-compatible way. Your existing test files do not need to be
rewritten, but you can achieve better results by adding Lua or switching
completely.

See {doc}`the usage documentation <../usage/lua-introduction>` for more!

## New probe sub-command

The new `probe` sub-command of `cloe-engine` allows you to probe a simulation
configuration with merged stack files.

In this mode, a simulation is set up and all the participants are connected,
probed, and then disconnected. That means:

- the simulation must be fully configured,
- failures may occur during connection phase, and
- scripts included must be correct.

The output of the probe is a JSON written to stdout:

```json
{
  "http_endpoints": [ "/endpoints", ... ],
  "plugins": {
    "PLUGIN_NAME": "PLUGIN_PATH",
    ...
  },
  "signals": {
    "SIGNAL_NAME": "DESCRIPTION",
    ...
  },
  "tests": {
  }
  "trigger_actions": [ "ACTION1", "..." ],
  "trigger_events": [ "EVENT1", ... ],
  "uuid": "UUID",
  "vehicles": {
    "NAME": [ "COMPONENT1", ... ]
  }
}
```

The tests section is only defined if the simulation configuration
contains Lua that defines tests (via `cloe.schedule_test()` API).
You can use tools like `jq` to further filter and refine the output.

Examples:

    cloe-engine probe tests/test_nop_smoketest.json
    cloe-engine probe my_lua_test.lua | jq -r '.tests | keys | .[]'

## Engine changes

- Add `--output-path` option to ``run`` command.

  This allows you to specify where output files should be written without having
  to configure it in the stackfile.

- Remove `--distinct` flag from ``check`` command.

  It wasn't used much and complicated the code. This old behavior can be easily
  achieved with outside tooling, such as `xargs` and `find`, so there is no loss
  in functionality overall.

## Tooling changes

The `cloe` Conan package was previously a meta-package containing all packages
created from this repository.

This package is being discontinued from this version of Cloe.
It was primarily meant for testing purposes, and thanks to new dependencies
in our project and the limitations of Conan 1, it no longer fulfills this role.

**Migration:**
> If you have been using the `cloe` package as a first-level dependency, then
> you will need to specify all the packages individually that you need.
>
> It is highly recommended to split your build Conanfile from your deployment
> Conanfile. For your controller build Conanfile, you generally only need:
>
>     def requirements(self):
>       def cloe_requires(dep):
>         self.requires(f"{dep}/0.25.0@cloe/develop")
>
>       cloe_requires("cloe-runtime")
>       cloe_requires("cloe-models")
>
> And for your deployment Conanfile you should only include the plugins you
> actually use:
>
>     def requirements(self):
>       def cloe_requires(dep):
>         self.requires(f"{dep}/0.25.0@cloe/develop")
>
>       cloe_requires("cloe-engine")
>       cloe_requires("cloe-plugin-clothoid-fit")
>       cloe_requires("cloe-plugin-frustum-culling")
>       cloe_requires("cloe-plugin-gndtruth-extractor")
>       cloe_requires("cloe-plugin-noisy-sensor")
>       cloe_requires("cloe-plugin-esmini")
>       self.requires("your-plugin")
>       self.requires("esmini-data/2.37.4@cloe/stable")
>
>       # Override as needed:
>       self.requires("esmini/2.37.4@cloe/stable", override=True)
>       self.requires("fmt/9.1.0", override=True)
>       self.requires("inja/3.4.0", override=True)
>       self.requires("nlohmann_json/3.11.3", override=True)
>       self.requires("incbin/cci.20211107", override=True)
>       self.requires("boost/1.74.0", override=True)
>       self.requires("zlib/1.2.13", override=True)
>       self.requires("protobuf/3.21.12", override=True)

For developing Cloe, the root Conanfile is now a super-build of Cloe, which
provides a vastly simplified workflow for editing Cloe code:

    echo "0.26.0-nightly" > VERSION
    make editable

And then iterate changes and (re-)builds with

    make all

This is a boon to development, as we make `cloe` editable and only have to work
with a single package. It also massively speeds up compilation:

- Conan and CMake configuration is only performed once.
- All cores can now be utilized much more effectively during the build process.

The main Makefile has been adjusted for this:

- `status` is now `status-all`
- `deploy` is now `deploy-all`
- `clean` is now `clean-all`
- `purge` is now `purge-all`

The following top-level Make targets just refer to `cloe` super-build package:

- `package`
- `smoketest`
- `smoketest-deps`
- `status`
- `export`

Finally, we moved all tests into project root tests directory.
We still support multiple profiles, but this makes it easier to support the
cloe super-build and should also make it easier to develop and maintain tests.

## Bazel Build

We have added experimental support for Bazel builds. Bazel is a promising
build system for enterprises, as it radically simplifies the build tooling
required. We have a single tool Bazel, instead of three or four layers of tools;
in our case: Make -> Conan -> CMake -> Make/Ninja.

The `.bazelrc.user` file needs to be configured in order for certain third-party
modules to be found (see repository `README.md`)

- `bazel build //...` builds everything in the project.
- `bazel test //...` runs all tests.
- `bazel run :cloe_shell` launches a shell wherein `cloe-engine` is available.

Full feature parity with Conan is not yet achieved. For example, there is
no Bazel build for VTD or ESmini yet. Contributions are welcomed! :-)

## Library changes

At various places, `boost::optional` and `boost::filesystem` types and methods are
replaced by their C++17 counterparts. This makes our libraries more lightweight
by reducing their dependence on Boost.
