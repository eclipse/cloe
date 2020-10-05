Example Configurations
======================

This directory contains several example configurations. These configurations
consist of complete and incomplete configuration files. The complete
configuration files include other (possibly incomplete) configuration files to
reduce the amount of duplication in configuration files. For example,
`config_nop_smoketest.json` includes the complete `config_nop_infinite.json`
stack file, and it in turn includes several incomplete `controller_*.json`
stack files.

## config_*.json

All stack files that are prefixed with `config_` are complete stack files that
can be checked or run with cloe-engine:

```console
$ build/cloe-engine run tests/config_nop_smoketest.json
...
```

## controller_*.json

The controller configuration is used in both NOP as-well-as VTD simulator
binding stack files. To reduce duplication, the controller specific
configuration is placed in its own partial configuration file. This file
can be checked, but it might fail:

```console
$ build/cloe-engine check tests/controller_basic.json
tests/controller_basic.json: cannot find a vehicle with the name 'default': no entity with that name has been defined
```

## debug_callgrind.json

This stack file can be included *in addition* to another complete stack file
to enable callgrind profiling:

```console
$ valgrind --tool=callgrind --instr-atstart=no build/cloe-engine run tests/config_nop_smoketest.json tests/debug_callgrind.json
...
```

Testing with BATS
=================

Testing stack files with Cloe is automated with the [Bash Automated Testing
System](https://github.com/sstephenson/bats) (BATS). This requires the *bats*
tool to be installed:

```bash
sudo apt-get install bats
```

Once installed, Bats can be used with individual files or with the entire
directory:

```console
$ bats tests/
 ✓ Assert check error on incomplete logging conf
 ✓ Assert run success with curl trigger
 ✓ Assert run failure with fail trigger
 ✓ Run test_gndtruth_smoketest.json stack file
 ✓ Assert check error on incomplete conf
 ✓ Assert run error on incomplete conf
 ✓ Run test_namespaced_smoketest.json stack file
 ✓ Assert check error with no binding
 ✓ Assert run error with no binding
 ✓ Run test_nop_smoketest.json stack file
 ✓ Assert check error with no vehicle
 ✓ Assert run error with no vehicle
 ✓ Compare test_replica_smoketest.json replica stack file
 ✓ Run test_nop_smoketest.json stack file
 ✓ Assert check error with unknown vehicle
 ✓ Assert run error with unknown vehicle
 ✓ Run test_vtd_smoketest.json stack file
 - Assert check error on unknown sensor reference (skipped: this offline check not implemented yet)
 ✓ Assert run error on unknown sensor reference

19 tests, 0 failures, 1 skipped
```

The test framework checks that Cloe has been compiled. If this is not the case,
it will compile Cloe for you, which will cause the first test to take roughly
2 minutes longer to complete.

## setup_bats*.bash

These files contain definitions that are used in all specific BATS scripts.
It should not be used directly, as it is loaded in each bats script.

## test_*.bats

Each `*.bats` file performs one or more tests, possibly on an equally named
JSON stack file.

## test_*.json

Each stack file tests a part of the Cloe runtime functionality, and is usually
accompanied by a Bats file.
