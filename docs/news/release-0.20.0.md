# Version 0.20.0 Release

This version includes several improvements to the simulator plugin for VTD
and an update to the stack file format, among many other smaller improvements.

For the entire changelog, see the [Git commit history](https://github.com/eclipse/cloe/compare/v0.19.0...v0.20.0).

## VTD Simulator Plugin

TLDR: The `vtd` simulator binding now has support for VTD 2022.3,
OpenScenario files, and the `scp` action to send SCP messages from
triggers.

The `vtd` simulator binding now has support for
[VTD 2022.3](https://mscsoftware.my.site.com/customers/s/article/Whats-New-VTD-2022-3)
and using OpenScenario.

The simulator binding can now be built with *one* of the two VTD versions
supported:

- VTD 2.2.0
- VTD 2022.3

For this, the respective `vtd` and `vtd-api` Conan package needs to
be available. These can be built from the `optional/vtd/vendor`
folder, provided you have the corresponding sources.

Then, simply build the `cloe-plugin-vtd` package with the
`vtd-api` dependency set to the version you want to use it with.

The `scp` action has been added, which lets you send SCP messages to VTD from
a trigger:
```json
{
    "event": "time=5",
    "action": {
        "name": "vtd/scp",
        "xml": "<SimCtrl><Stop/></SimCtrl>"
    }
}
```

You can also define SCP templates in the `vtd` binding configuration:
```yaml
version: "4"
defaults:
  simulators:
    binding: vtd
    args:
      scp_actions:
        simctrl: "<SimCtrl><[[cmd]]/></SimCtrl>"
        stop: >
            <SimCtrl>
                <Stop/>
            </SimCtrl>
```
This can then be used without arguments like so:
```json
{
  "event": "time=5",
  "action": "vtd/scp=stop"
}
```

Or with template arguments in the long form:
```json
{
  "event": "time=5",
  "action": {
    "name": "vtd/scp",
    "template": "simctrl",
    "data": {
      "cmd": "Stop"
    }
  }
}
```
See the plugin {doc}`documentation <../reference/plugins/vtd>` for more details.

## Support for Vehicle Dynamics Model Integration

TLDR: You can now integrate custom vehicle dynamics models as Cloe
plugins. The updated ego vehicle state will be sent to the `vtd` simulator, if
configured accordingly.

A new interface `VehicleStateModel` has been added to `models`. This can be
used as base class for custom vehicle dynamics model plugins. User models shall
provide the updated ego vehicle state as `Object` type in every simulation
cycle.

An additional interface `DriverRequest` has been added to pass requests from
a driver model (built into the simulator or external) to a controller plugin.
The `basic` controller has been extended to forward requests from the
configured `DriverRequest` to the `actutator` component, in case no ADAS
function is active. Albeit being a simplistic arbitration logic, this ensures
that a custom vehicle dynamics model that reads from the `actuator` component
will always receive a valid actuation request.

The `vtd` binding now supports sending the updated ego vehicle state from
a custom vehicle dynamics model to the simulator. Driver steering and
acceleration requests from the `vtd` driver model are received by the plugin.

## Stackfile 4.1

TLDR: The stackfile schema format has been updated to support minor versions.

That means for this version of the Cloe Engine, the following versions all mean
the same thing: `4`, `4.0`, `4.1`. The default version is the *string* `4.1`:
```json
{
  "version": "4.1"
}
```
If they are all compatible, you may wonder what the point is. The key phrase is
"for this version of the engine". An older version of the engine will reject
the stackfile, and the current version will reject a version that is newer
than `4.1`. That means we can effectively specify the minimum stack file
version required, if we need to be explicit. This follows the way that tools
like `docker-compose` have managed versioning the input files.

If you don't care, just keep using the version `4`. Nothing will change for you.
If on the other hand, you need to ensure that a recent version of Cloe is
used, because you are using an added feature, then you should specify the
minor version that introduced the feature you want to use, or simply the latest
one.

Apart from validation, this has no other effect on parsing or the simulation.

## Optional Triggers

TLDR: Triggers can have an `optional: true` field set to make them not fail
when the action or event does not exist.

This is the change that caused the stackfile to be updated and version bumped.

Nominally, triggers consist of an *event* and an *action*. There are several
other fields that can be specified to modify specifics of how Cloe treats
the trigger. The new `optional` field is false by default and specifies
whether the trigger can be ignored if it cannot be constructed.

That is, if construction of a trigger fails because the specified event and/or
action are not available, then a warning is printed instead of a simulation
abort.

When is this useful, you may ask. When a controller plugin has actions that are
generated by some code-generation routine that can be configured, then different
configurations can result in a different set of actions. To support using the
same trigger list with these different configurations, the "problematic"
triggers can be made optional.

Since this is still not an optimal solution, warnings are still printed when
a trigger is discarded like this.

## Fable Improvements

TLDR: The fable library has been refactored and may require you to include
`fable/schema/number_impl.hpp` or `fable/schema.hpp` for your code to keep
compiling. Compile performance should be better though.

There are these changes to the Fable library:

New:

- Add String schema `enum_of` method to specify valid values of string.
- Add String schema unit tests to test all features.
- Extend utility header `gtest.hpp` to support Schema type where previously
  only Confable type was supported.

Changed:

- Remove include of `fable/schema.hpp` in `fable/confable.hpp`.
  This allows better compile-time optimization but may require users to include
  `fable/schema.hpp` themselves.
- Update example projects to use modern CMake commands in Makefile.
- Update Conan test package test_v2_package to support
  `conan create .` outside of test package mode.

Performance:

- Extract String implementation from header.
- Extract Boolean implementation from header.
- Extract Number implementation from header.
- If a user uses a Number with a non-standard numeric type, then they need to
  include `fable/schema/number_impl.hpp`

## Conan 2.0 Support

TLDR: The tooling in the project has been updated for better compatibility with
Conan 2.0 and packages that use more recent Conan features.

These changes allow for the following, among other things:

Using standard CMake commands such as find_package() for dependency management.
Use of multiple build types at the same time, such as Release and Debug.
Relying on a single lockfile for CI, once Conan 2.0 is used.

## Smoketest Conanfiles

TLDR: Smoketest configurations are named `conanfile_default.py` instead of
`profile_default.py` since the latter terminology conflicts with Conan.
This change only really affects those developing on or with Cloe.

This version contains changes in how the test profiles are named since the
original name was misleading.
The files in question were all located in `tests/` directories alongside
stackfiles and BATS tests and were previously named `profile_XYZ.py`.

Specifically, it was unclear when talking about the profile test files if the
user wanted to refer about the conan profile instead. To avoid this confusion
we decided to rename the files to a more understandable name: `conanfile_XYZ.py`.
This makes clear that these are normal conanfiles.
